#include "speaker.h"
#include "../../config.h"

// ==========================================
// 🔊 基础提示音与系统旋律驱动实现
// ==========================================

// --- 音符频率定义 (部分经典音阶) ---
#define NOTE_C5  523
#define NOTE_E5  659
#define NOTE_G5  784
#define NOTE_C6  1047

// --- 旋律音符结构体 ---
struct Note {
    int frequency; // 频率 (Hz)
    int duration;  // 持续时间 (ms)
};

// [为什么不用真实 MIDI 解析？]
// 真实的 Standard MIDI File (.mid) 包含复杂的区块头、轨道、Delta-Time 和多通道乐器映射。
// 引入一个完整的 MIDI 解析库去解析几秒钟的开机音效，不仅极大地消耗 Flash 和 RAM，
// 而且极易与 LVGL 争抢 CPU 资源。采用简单的“频率+时长”结构体数组，
// 是嵌入式系统中最复古、最轻量、最安全的做法。
const Note aboutMelody[] = {
    {NOTE_C5, 300}, 
    {NOTE_E5, 300}, 
    {NOTE_G5, 300}, 
    {NOTE_C6, 1200} // 模拟 Win95/98 经典的空灵上扬和长余音，总长约 2.1 秒
};
const int MELODY_LENGTH = sizeof(aboutMelody) / sizeof(aboutMelody[0]);

// --- 内部状态控制变量 ---
static bool isMelodyPlaying = false;
static int currentNoteIndex = 0;
static unsigned long noteStartTime = 0;

static bool isBeeping = false;
static unsigned long beepStopTime = 0;

// ==========================================
// [接口] 初始化扬声器
// ==========================================
bool speakerBegin() {
    // 检查 config.h 中的引脚分配
    if (SPEAKER_PIN != -1) {
        pinMode(SPEAKER_PIN, OUTPUT);
        digitalWrite(SPEAKER_PIN, LOW);
    }
    Serial.println("[成功] 基础提示音与旋律模块初始化完毕");
    return true;
}

// ==========================================
// [接口] 触发短促提示音 (非阻塞)
// ==========================================
void speakerBeep(int durationMs) {
    if (SPEAKER_PIN == -1) return; // 未配置硬件则安全跳过
    
    tone(SPEAKER_PIN, 2000); // 发生 2kHz 的清脆滴声
    beepStopTime = millis() + durationMs;
    isBeeping = true;
}

// 强制开启发声
void speakerOn() {
    if (SPEAKER_PIN != -1) tone(SPEAKER_PIN, 1000);
}

// 强制关闭发声
void speakerOff() {
    if (SPEAKER_PIN != -1) noTone(SPEAKER_PIN);
}

// ==========================================
// [接口] 触发 About 页面旋律
// ==========================================
// [为什么旋律只播一次？]
// 遵循《ESPdesktop 项目总宪章》的音频规则，进入 About 页面是为了体验
// 经典的仪式感，循环播放会严重干扰用户阅读关于信息并显得廉价。
void speakerPlayAboutMelody() {
    if (isMelodyPlaying) return; // 如果正在播放，则不重复触发

    isMelodyPlaying = true;
    currentNoteIndex = 0;
    noteStartTime = millis();

    if (SPEAKER_PIN != -1) {
        tone(SPEAKER_PIN, aboutMelody[0].frequency);
    }
}

// ==========================================
// [接口] 状态查询
// ==========================================
bool speakerMelodyIsPlaying() {
    return isMelodyPlaying;
}

// ==========================================
// [泵] 旋律与提示音的非阻塞调度泵
// ==========================================
// [为什么旋律必须非阻塞？]
// 如果在播放音符时使用 delay(300)，在这 300 毫秒内，主程序的 loop() 将完全停滞，
// LVGL 的动画会瞬间卡住，用户的摇杆按键也不会有任何反应。
// 使用 millis() 进行状态机切换，就像电影放映机一样，每次循环只检查“是否该换下个音了”，
// 检查完毕立刻交出 CPU 控制权，保障 UI 丝滑运行。
void speakerMelodyLoop() {
    unsigned long currentMillis = millis();

    // 1. 处理短 Beep 的超时关闭
    if (isBeeping && currentMillis >= beepStopTime) {
        isBeeping = false;
        // 如果旋律没在播放，才真正关闭硬件声音，防止互相掐架
        if (!isMelodyPlaying && SPEAKER_PIN != -1) {
            noTone(SPEAKER_PIN);
        }
    }

    // 2. 处理旋律的音符推进
    if (isMelodyPlaying) {
        if (currentMillis - noteStartTime >= aboutMelody[currentNoteIndex].duration) {
            // 当前音符时长已到，推进到下一个
            currentNoteIndex++;
            
            if (currentNoteIndex >= MELODY_LENGTH) {
                // 整个数组播放完毕
                isMelodyPlaying = false;
                if (SPEAKER_PIN != -1 && !isBeeping) {
                    noTone(SPEAKER_PIN);
                }
            } else {
                // 播放下一个新音符
                noteStartTime = currentMillis;
                if (SPEAKER_PIN != -1) {
                    tone(SPEAKER_PIN, aboutMelody[currentNoteIndex].frequency);
                }
            }
        }
    }
}