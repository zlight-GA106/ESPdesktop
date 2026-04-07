#pragma once

// ==========================================
// 🎮 调试模式开关
// ==========================================
#define FAKE_DATA_MODE      1    // 假数据模式：1=启用,0=禁用 (UI调试用)

// ==========================================
// 🚀 ESPdesktop 全局硬件与参数配置表
// 适用硬件：ESP32-S3-DevKitC-1 (N16R8)
// ==========================================

// ==========================================
// 【1】 屏幕基础参数 (ST7735S SPI)
// ==========================================
#define TFT_WIDTH           128
#define TFT_HEIGHT          160

// 物理引脚连接
#define TFT_SCLK            12    // SPI Clock
#define TFT_MOSI            11    // SPI Data Out (SDA)
#define TFT_MISO            -1    // 屏幕无需回传数据
#define TFT_CS              14    // Chip Select
#define TFT_DC              9     // Data/Command
#define TFT_RST             10    // Reset
#define TFT_BL              13    // 背光控制 (BLK)

// ==========================================
// 【2】 I2C 总线配置 (传感器共用)
// ==========================================
#define I2C_SCL             8
#define I2C_SDA             18

// 传感器设备地址
#define SHT40_I2C_ADDR      0x44  // 温湿度传感器
#define BH1750_I2C_ADDR     0x23  // 光照传感器

// 兼容旧命名
#define LIGHT_SENSOR_ADDR   BH1750_I2C_ADDR

// ==========================================
// 【3】 五向摇杆按键定义 (GPIO 输入)
// 适配 ui_keypad.cpp 中的 KEY_ 系列宏
// ==========================================
#define KEY_UP_PIN          38
#define KEY_DOWN_PIN        37
#define KEY_LEFT_PIN        36
#define KEY_RIGHT_PIN       35
#define KEY_OK_PIN          47    // 中间确认键 (MID)

// ==========================================
// 【4】 音频输出 (I2S DAC - MAX98357A)
// 适配 audio_player.cpp
// ==========================================
#define I2S_DOUT_PIN        6     // Data Out
#define I2S_BCLK_PIN        4     // Bit Clock
#define I2S_LRC_PIN         5     // Left/Right Clock (WS)

// 兼容旧命名 (无 _PIN 后缀)
#define I2S_DOUT            I2S_DOUT_PIN
#define I2S_BCLK            I2S_BCLK_PIN
#define I2S_LRCLK           I2S_LRC_PIN

// ==========================================
// 【4b】 扬声器直接驱动（可选）
// 如果使用 tone() 函数而非 I2S DAC
// ==========================================
#define SPEAKER_PIN         3     // 扬声器 GPIO (或 -1 禁用)

// ==========================================
// 【5】 麦克风输入 (I2S Mic - INMP441)
// 适配 microphone.cpp
// ==========================================
#define MIC_I2S_SD_PIN      17    // Serial Data
#define MIC_I2S_SCK_PIN     16    // Serial Clock
#define MIC_I2S_WS_PIN      15    // Word Select
#define MIC_I2S_LR_PIN      7     // L/R 选择

// 兼容旧命名
#define MIC_SD_PIN          MIC_I2S_SD_PIN
#define MIC_SCK_PIN         MIC_I2S_SCK_PIN
#define MIC_WS_PIN          MIC_I2S_WS_PIN

// ==========================================
// 【6】 人体雷达配置 (UART - HLK-LD2402)
// ==========================================
#define RADAR_RX_PIN        1     // 接雷达 TX
#define RADAR_TX_PIN        2     // 接雷达 RX
#define RADAR_BAUD          115200

// ==========================================
// 【7】 RGB 氛围灯 (WS2812B)
// ==========================================
#define RGB_DATA_PIN        48    // 单线控制引脚
#define RGB_LED_COUNT       1     // 灯珠数量

// ==========================================
// 【8】 系统逻辑阈值与默认值
// ==========================================
// 判定阈值
#define TEMP_HIGH_THRESHOLD   35.0f // 温度警告线 (°C)
#define LIGHT_LOW_THRESHOLD   30    // 自动变暗阈值 (Lux)
#define SOUND_HIGH_THRESHOLD  2000  // 噪音判定值 (原始采样值)

// UI 默认参数
#define DEFAULT_BRIGHTNESS    200   // 0-255 (针对 ST7735 BL 引脚)
#define DEFAULT_VOLUME        15    // 0-30 (I2S 增益或软件音量)
#define UI_ANIM_DURATION      250   // 页面切换动画耗时 (ms)

// ==========================================
// 【9】 调试配置
// ==========================================
#define SERIAL_BAUD         115200
#define DEBUG_MODE          true    // 是否开启串口调试输出