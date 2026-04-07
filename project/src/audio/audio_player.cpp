#include "audio_player.h"
#include "../../config.h"

// 引入 ESP8266Audio 核心组件
#include <AudioGeneratorMP3.h>
#include <AudioGeneratorWAV.h>
#include <AudioFileSourceICYStream.h>
#include <AudioFileSourcePROGMEM.h>
#include <AudioOutputI2S.h>

// ==========================================
// 🎵 高级音频流与音效播放器实现 (MAX98357A - I2S)
// ==========================================

// --- 播放器内部状态枚举 ---
enum PlaybackState {
    STATE_IDLE,       // 空闲状态
    STATE_STREAMING,  // 正在播放持续性音频 (Radio 电台 或 AI_TTS)
    STATE_UI_SFX      // 正在播放 UI 瞬时音效 (click.wav)
};

// 内部核心指针
static AudioGenerator *decoder = nullptr;      // 音频解码器 (MP3 / WAV)
static AudioFileSource *source = nullptr;      // 数据源 (网络流 / 内存)
static AudioOutputI2S *output = nullptr;       // I2S 输出通道

static PlaybackState currentState = STATE_IDLE;

// ==========================================
// ⚠️ 占位：极简的 click.wav 十六进制数组
// 真实的按键音需通过 xxd 或网页工具转换后覆盖此处
// ==========================================
const unsigned char click_wav_data[] PROGMEM = {
    0x52, 0x49, 0x46, 0x46, 0x24, 0x00, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45, 
    0x66, 0x6d, 0x74, 0x20, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 
    0x44, 0xac, 0x00, 0x00, 0x88, 0x58, 0x01, 0x00, 0x02, 0x00, 0x10, 0x00, 
    0x64, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x00 // 极简占位头部
};

// ==========================================
// [流程] 初始化 I2S 音频引擎
// ==========================================
bool audioPlayerBegin() {
    // 强制使用 I2S_NUM_1 (因为 I2S_NUM_0 已经分配给了 INMP441 麦克风，防止冲突)
    output = new AudioOutputI2S(1, AudioOutputI2S::EXTERNAL_I2S);
    
    // 绑定 config.h 中定义的 MAX98357A 引脚
    // 参数顺序: BCLK, LRC, DOUT
    output->SetPinout(I2S_BCLK, I2S_LRCLK, I2S_DOUT);
    
    // 初始化默认音量 (由外层系统接管，此处设为适中)
    output->SetGain(0.5f); 

    Serial.println("[成功] I2S 高级音频播放引擎已就绪 (通道 1)");
    return true;
}

// ==========================================
// [流程] 立即停止当前播放
// ==========================================
void audioPlayerStop() {
    // [为什么 click.wav 不属于页面上下文持续音频？]
    // click.wav 是短促的 UI 交互音效，生命周期极短，应该自然播放完毕。
    // ui_manager 在切换页面（如退出 RadioPage）时会无差别调用 stop()，
    // 如果这时候刚好 click 还在发声，直接掐断会导致“咔哒”一声尖锐的破音。
    // 因此：如果当前状态是 UI 音效，直接无视停止指令，让子弹飞一会儿。
    if (currentState == STATE_UI_SFX) {
        return; 
    }

    // [为什么 stop 必须可靠？]
    // 离开电台或 AI 页面时，如果不彻底销毁解码器和网络流对象，
    // ESP32 的内存（Heap）会瞬间泄漏，网络 Socket 也会被死死占用，
    // 导致设备几秒钟后直接 Watchdog Reset（看门狗重启）。
    if (decoder) {
        decoder->stop();
        delete decoder;
        decoder = nullptr;
    }
    if (source) {
        source->close();
        delete source;
        source = nullptr;
    }
    
    currentState = STATE_IDLE;
    Serial.println("[音频] 已彻底中断持续性流媒体播放，释放内存");
}

// ==========================================
// [流程] 播放网络流媒体 (Radio / TTS)
// ==========================================
bool audioPlayerPlayUrl(const String &url) {
    // 播放新流之前，必须绝对可靠地掐断旧流
    audioPlayerStop();

    Serial.printf("[音频] 正在连接网络流媒体: %s\n", url.c_str());

    // 使用支持 ICY (Shoutcast) 协议的网络流读取器，兼容绝大部分电台
    source = new AudioFileSourceICYStream(url.c_str());
    decoder = new AudioGeneratorMP3();
    
    if (decoder->begin(source, output)) {
        currentState = STATE_STREAMING;
        return true;
    } else {
        Serial.println("[错误] 流媒体解码失败或 URL 无效");
        audioPlayerStop();
        return false;
    }
}

// ==========================================
// [流程] 播放 UI 点击音效
// ==========================================
void audioPlayerPlayClick() {
    // 为了保证项目的“最小可运行版本”稳定：
    // 如果当前正在播放电台流媒体，播放 click 会争抢唯一的 I2S 解码通道。
    // 重新缓冲网络流代价太大，所以当处于流媒体状态时，我们直接忽略按键音。
    if (currentState == STATE_STREAMING) {
        return; 
    }

    // 确保通道干净
    audioPlayerStop();

    // 从 PROGMEM (Flash 闪存) 中直接读取 C-Array，零延迟！
    source = new AudioFileSourcePROGMEM(click_wav_data, sizeof(click_wav_data));
    decoder = new AudioGeneratorWAV();
    
    if (decoder->begin(source, output)) {
        currentState = STATE_UI_SFX;
    }
}

// ==========================================
// [查询] 状态返回
// ==========================================
bool audioPlayerIsPlaying() {
    return (currentState != STATE_IDLE);
}

// ==========================================
// [核心泵] 非阻塞音频数据推进
// ==========================================
void audioPlayerLoop() {
    // 如果解码器正在运行，非阻塞地喂入数据
    if (decoder && decoder->isRunning()) {
        if (!decoder->loop()) {
            // 一旦 loop() 返回 false，说明音频播放完毕（如 TTS 结束，或 click 播完）
            // 立即触发硬停止，安全回收内存
            decoder->stop();
            
            delete decoder; decoder = nullptr;
            delete source; source = nullptr;
            
            currentState = STATE_IDLE;
        }
    }
}