#include "temp_humi.h"
#include "../../config.h"
#include <Wire.h>
#include <Adafruit_SHT4x.h>

// ==========================================
// 🌡️ 温湿度传感器驱动实现 (SHT40)
// ==========================================

// 实例化 Adafruit 的 SHT4x 驱动对象
Adafruit_SHT4x sht4 = Adafruit_SHT4x();

// 全局标志位，记录传感器的在线状态
static bool isSht4Connected = false;

// ==========================================
// [流程] 初始化传感器
// ==========================================
bool tempHumiBegin() {
#if FAKE_DATA_MODE
    // [假数据模式] 跳过硬件初始化，直接标记为已连接
    Serial.println("[DEBUG] SHT40 温湿度传感器 (假数据模式)");
    isSht4Connected = true;
    return true;
#else
    // 1. 初始化 I2C 总线，使用 config.h 中定义的全局引脚
    Wire.begin(I2C_SDA, I2C_SCL);

    // 2. 尝试连接 SHT40
    // sht4.begin() 会在 I2C 总线上寻找地址为 0x44 的芯片
    if (!sht4.begin(&Wire)) {
        // [失败处理] 如果找不到芯片，打印警告，并标记为断线
        Serial.println("[错误] 找不到 SHT40 传感器，请检查 SDA(18) / SCL(8) 接线！");
        isSht4Connected = false;
        return false;
    }

    // 3. 配置传感器参数 (极简模式：高精度，不开启内部除霜加热器)
    sht4.setPrecision(SHT4X_HIGH_PRECISION);
    sht4.setHeater(SHT4X_NO_HEATER);

    Serial.println("[成功] SHT40 温湿度传感器初始化完毕");
    isSht4Connected = true;
    return true;
#endif
}

// ==========================================
// [流程] 读取温湿度数据
// ==========================================
bool tempHumiRead(float &temperature, float &humidity) {
    // 如果初始化都没过，直接拦截，防止底层 I2C 阻塞死机
    if (!isSht4Connected) {
        return false;
    }

#if FAKE_DATA_MODE
    // [假数据模式] 生成虚拟数据，模拟室温变化
    static bool fakeDir = true;
    static float fakeTemp = 22.5f;
    // 每次读取时温度缓慢变化 ±0.1°C，模拟真实波动
    fakeTemp += fakeDir ? 0.1f : -0.1f;
    if (fakeTemp > 28.0f) fakeDir = false;
    if (fakeTemp < 18.0f) fakeDir = true;
    
    temperature = fakeTemp;
    humidity = 45.0f + random(-5, 5);  // 45% ±5% 相对湿度
    return true;
#else
    // 声明 Adafruit 专用的传感器事件结构体
    sensors_event_t humidityEvent, tempEvent;

    // 获取最新的物理测量值
    if (sht4.getEvent(&humidityEvent, &tempEvent)) {
        // [成功处理] 将读到的真实数据，赋值给主程序传进来的变量 (通过引用 &)
        temperature = tempEvent.temperature;
        humidity = humidityEvent.relative_humidity;
        return true;
    } else {
        // [失败处理] 如果读取过程中 I2C 通信受到干扰，返回 false
        // 注意：这里不要去把 temperature 设为 0，让外部继续用上一次的旧数据，避免 UI 闪烁
        Serial.println("[警告] SHT40 数据读取失败");
        return false;
    }
#endif
}

// ==========================================
// [流程] 返回在线状态
// ==========================================
bool tempHumiIsConnected() {
    return isSht4Connected;
}