#include "microphone.h"
#include "../../config.h"
#include <driver/i2s.h>

// ==========================================
// 🎤 麦克风采集驱动实现 (INMP441 - I2S)
// ==========================================

// 使用 I2S 通道 0 (将通道 1 留给后续的扬声器使用，防止冲突)
#define I2S_MIC_PORT I2S_NUM_0

// 保存最后一次读取的原始值
static int lastRawValue = 0;

// 用于平滑处理的包络值 (EMA 算法状态)
static float smoothedLevel = 0.0f;

// ==========================================
// [流程] 初始化麦克风 (I2S DMA 配置)
// ==========================================
bool microphoneBegin() {
#if FAKE_DATA_MODE
    // [假数据模式] 跳过硬件配置
    Serial.println("[DEBUG] INMP441 麦克风 (假数据模式)");
    return true;
#else
    // 1. I2S 工作参数配置
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX), // 主机接收模式
        .sample_rate = 16000,                                // 采样率 16kHz (对于噪音检测足够了)
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,        // INMP441 实际是 24-bit，放入 32-bit 槽中
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,         // 仅读取左声道 (模块 L/R 引脚接地)
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,   // 标准 I2S 协议
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,            // 默认中断优先级
        .dma_buf_count = 4,                                  // DMA 缓冲区数量
        .dma_buf_len = 256,                                  // 每个缓冲区的采样点数
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    // 2. 引脚映射配置 (从 config.h 中获取)
    i2s_pin_config_t pin_config = {
        .bck_io_num = MIC_SCK_PIN,
        .ws_io_num = MIC_WS_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE, // 麦克风不需要输出引脚
        .data_in_num = MIC_SD_PIN
    };

    // 3. 安装驱动并设置引脚
    if (i2s_driver_install(I2S_MIC_PORT, &i2s_config, 0, NULL) != ESP_OK) {
        Serial.println("[错误] 麦克风 I2S 驱动安装失败！");
        return false;
    }
    if (i2s_set_pin(I2S_MIC_PORT, &pin_config) != ESP_OK) {
        Serial.println("[错误] 麦克风 I2S 引脚映射失败！");
        return false;
    }

    Serial.println("[成功] INMP441 麦克风初始化完毕");
    return true;
#endif
}

// ==========================================
// [接口] 读取原始瞬时采样值 (Raw)
// ==========================================
// [说明]：Raw 值是正负交替的声波瞬时振幅。如果此时没人说话，它在 0 附近极小波动；
// 一旦有声音，它会以 16000次/秒 的极快速度在正负数（如 -8000 到 +8000）之间疯狂跳变。
int microphoneReadRaw() {
    int32_t sample = 0;
    size_t bytesRead = 0;
    
    // 从 DMA 缓冲区中拿 1 个样本 (4字节)
    i2s_read(I2S_MIC_PORT, &sample, sizeof(int32_t), &bytesRead, portMAX_DELAY);
    
    if (bytesRead > 0) {
        // INMP441 的数据是 24 位的，且在 32 位槽中左对齐。
        // 右移 14 位，将其缩放至一个适合普通 int 处理的区间，同时过滤掉底噪位。
        lastRawValue = sample >> 14; 
    }
    return lastRawValue;
}

// ==========================================
// [接口] 读取平滑后的声音强度 (Level) 适合 UI 与逻辑判定
// ==========================================
// [说明]：Level 值提取了这段时间声波的“绝对值峰值”，并通过 EMA 平滑算法
// 过滤掉了瞬间的杂音刺突。它变成了一个单向增长的正数，非常适合作为
// “当前环境有多吵”的指标（类似于音量条进度），喂给 UI 或判断是否触发噪音报警。
int microphoneReadLevel() {
#if FAKE_DATA_MODE
    // [假数据模式] 生成虚拟声音强度，模拟环境噪音
    static unsigned long lastFakeSoundTime = 0;
    static int fakeLevel = 20;
    unsigned long now = millis();
    
    // 每 2000ms 改变一次声音强度
    if (now - lastFakeSoundTime > 2000) {
        lastFakeSoundTime = now;
        // 在 10-50 dB 之间随机波动，模拟环境噪音变化
        fakeLevel = 10 + random(0, 41);
    }
    return fakeLevel;
#else
    int32_t samples[64]; // 一次性读取 64 个样本进行统计分析 (时间跨度约为 4ms)
    size_t bytesRead = 0;
    
    // 批量读取数据块
    i2s_read(I2S_MIC_PORT, samples, sizeof(samples), &bytesRead, portMAX_DELAY);
    
    int samplesRead = bytesRead / sizeof(int32_t);
    int peak = 0;
    
    // 1. 寻找这 64 个样本中的振幅峰值 (求绝对值最大值)
    for (int i = 0; i < samplesRead; i++) {
        int val = samples[i] >> 14;
        if (val < 0) val = -val; // 翻转负半轴
        if (val > peak) peak = val;
    }

    // 2. 指数移动平均 (EMA) 平滑算法
    // 0.2 的新数据权重 + 0.8 的历史权重，让数值变化像加上了阻尼器一样平滑
    smoothedLevel = (0.2f * peak) + (0.8f * smoothedLevel);

    // 3. 映射到 0~100 的直观区间 (假设 50.0 对应极其吵闹的环境)
    int level = (int)(smoothedLevel / 50.0f);
    
    // 限制上下界防溢出
    if (level > 100) level = 100;
    if (level < 0) level = 0;

    return level;
#endif
}