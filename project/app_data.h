#pragma once
#include <Arduino.h>
#include <stdint.h>
#include "src/ui/ui_i18n.h"   // UiLang 定义在此，避免重复声明

// ==========================================
// 💾 ESPdesktop 全局数据结构与状态中心
// ==========================================

// --- UI 页面 ID 枚举 (全系统唯一户口本) ---
// 注意：这里的命名必须和你在各 Page.cpp 里的 switch-case 保持一致
enum UiPageId {
    HOME_PAGE = 0,       // 0: 主页
    MENU_PAGE,           // 1: 主菜单
    SENSOR_PAGE,         // 2: 传感器详情页
    SETTINGS_MENU_PAGE,  // 3: 设置菜单
    WIFI_INFO_PAGE,      // 4: Wi-Fi 状态页
    BLUETOOTH_INFO_PAGE, // 5: 蓝牙页
    MQTT_INFO_PAGE,      // 6: MQTT 信息页
    BRIGHTNESS_PAGE,     // 7: 亮度调节页
    VOLUME_PAGE,         // 8: 音量调节页
    RADIO_PAGE,          // 9: 网络电台页
    AI_PAGE,             // 10: AI 助手页
    ABOUT_PAGE,          // 11: 关于本机
    DINO_PAGE,           // 12: 彩蛋游戏
    LANGUAGE_PAGE,       // 13: 语言设置
    PLACEHOLDER_PAGE     // 14: 占位
};

// ==========================================
// 【1】传感器实时数据池 (Sensor Data)
// ==========================================
struct SensorData {
    float temperature = 0.0f;
    float humidity = 0.0f;
    float lightLevel = 0.0f;       // 统一名称与 web_server.cpp 对齐
    bool  humanDetected = false;   // 统一名称与 web_server.cpp / radar 对齐
    int   soundLevel = 0;
};

// ==========================================
// 【2】系统运行状态与设置池 (System State)
// ==========================================
struct SystemState {
    // --- 网络状态 ---
    bool wifiConnected = false;
    char wifiIp[16] = "0.0.0.0";   // 对齐 project.ino 的 strncpy
    char wifiMask[16] = "0.0.0.0"; // 子网掩码，用于 ui_wifi_info_page.cpp
    char wifiMac[20] = "00:00:00:00:00:00";  // MAC 地址，用于 ui_wifi_info_page.cpp
    bool mqttConnected = false;
    char mqttBroker[64] = "";      // 对齐 ui_mqtt_info_page.cpp

    // --- AI 状态 ---
    bool aiBusy = false;
    int  aiState = 0;              // 0:待机, 1:思考, 2:播报

    // --- 时间日期 (用于 Home Page 渲染) ---
    int year = 2026;
    int month = 3;
    int day = 25;
    int hour = 0;
    int minute = 0;
    int second = 0;

    // --- UI 与硬件设置 ---
    int currentPageId = HOME_PAGE;
    int brightness = 80;
    int volume = 50;
    UiLang language = LANG_ZH_CN;  // 运行时语言（不持久化）

    // --- 氛围灯颜色 (用于 Sensor Page 显示) ---
    uint8_t rgbR = 0;
    uint8_t rgbG = 0;
    uint8_t rgbB = 0;

    // --- 音频通道状态 ---
    bool audioPlaying = false;
    bool radioPlaying = false;
    bool ttsPlaying = false;
    bool aboutMelodyPlaying = false;
};

// 全局变量外部声明，确保各模块访问同一块内存
extern SensorData g_sensorData;
extern SystemState g_systemState;