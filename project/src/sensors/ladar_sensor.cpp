#include "radar_sensor.h"
#include "../../config.h"

// ==========================================
// 🚶‍♂️ 人体存在雷达驱动实现 (HLK-LD2402 串口型)
// ==========================================

// 保存外部传入的硬件串口指针 (例如指向 Serial1)
static HardwareSerial* radarSerial = nullptr;

// 记录最新的解析结果：是否有人
static bool isPresenceDetected = false;

// ---------------------------------------------------------
// [核心逻辑说明：串口型为什么要 update / read 分离？]
// 1. 如果是 GPIO 型（如传统的红外 PIR），只需一个 digitalRead(PIN) 
//    就能瞬间知道高低电平，代码只有一行，极度简单。
// 2. 但串口雷达吐出的是一串连续的 16 进制字节流（帧）。如果在 read() 
//    里用 while 死等一整帧收完，会严重阻塞主循环，导致屏幕卡死。
// 3. 因此必须分离：
//    - radarUpdate()：放在 loop() 里高频调用，每次只把当前缓冲区里
//      刚到的几个字节“吃”掉，拼装好后更新内存变量，绝不等待。
//    - radarReadPresence()：瞬间返回内存变量的值，毫无延迟。
// ---------------------------------------------------------

// ==========================================
// [流程] 初始化雷达
// ==========================================
bool radarBegin(HardwareSerial &serialPort) {
    // 绑定物理串口
    radarSerial = &serialPort;
    isPresenceDetected = false;
    
#if !FAKE_DATA_MODE
    // 注意：真正的 Serial1.begin(115200, SERIAL_8N1, RX, TX) 
    // 会在主程序 project.ino 中统一配置，这里只负责接收对象引用。
#endif
    Serial.println("[成功] LD2402 雷达串口驱动已挂载");
    return true;
}

// ==========================================
// [流程] 串口数据泵 (非阻塞解析)
// ==========================================
bool radarUpdate() {
#if FAKE_DATA_MODE
    // [假数据模式] 定期切换人体状态，模拟真实检测
    static unsigned long lastFakeTime = 0;
    static bool fakeDetection = false;
    unsigned long now = millis();
    
    // 每 8000ms 切换一次人体检测状态
    if (now - lastFakeTime > 8000) {
        lastFakeTime = now;
        fakeDetection = !fakeDetection;
        isPresenceDetected = fakeDetection;
        return true;  // 模拟新帧
    }
    return false;
#else
    // 防御性拦截：串口未绑定则直接退出
    if (radarSerial == nullptr) return false;

    bool hasNewFrame = false;

    // 只要硬件缓冲区里有数据，就一直读，直到读空为止
    while (radarSerial->available() > 0) {
        uint8_t incomingByte = radarSerial->read();

        // ====================================================
        // ⚠️ 占位解析结构 (Minimal Runnable Skeleton)
        // ====================================================
        // HLK-LD2402 的真实协议包含复杂的帧头(如 0xF4 0xF3)、
        // 距离数据、能量数据以及帧尾。
        // 为了保证当前骨架的最小可运行性，这里采用极简占位逻辑：
        // 假设我们只捕获特定字节，例如 0x01 代表有人，0x00 代表无人。
        //
        // TODO: 后续应查阅海凌科 LD2402 官方协议手册，将此处替换为
        // 严谨的 switch-case 状态机 (State Machine) 进行帧头/尾校验。
        // ====================================================

        if (incomingByte == 0x01) {
            isPresenceDetected = true;
            hasNewFrame = true;
        } else if (incomingByte == 0x00) {
            isPresenceDetected = false;
            hasNewFrame = true;
        }
    }

    // 如果成功解析出完整一帧，返回 true 告知外部可以更新数据了
    return hasNewFrame;
#endif
}

// ==========================================
// [流程] 读取状态
// ==========================================
bool radarReadPresence() {
    return isPresenceDetected;
}