#include "settings.h"
#include <Preferences.h>
#include <string.h>
#include <stdio.h>

// ==========================================
// 💾 本地持久化配置模块实现
// ==========================================

// 实例化 Preferences 对象
Preferences prefs;

// 实例化全局配置实体，占用实际内存
AppSettings globalSettings;

// ==========================================
// [接口] 获取配置项引用
// 作用：让其他模块能够拿到 globalSettings 的内存地址进行读写
// ==========================================
AppSettings& settingsGet() {
    return globalSettings;
}

// ==========================================
// [流程] Load 流程
// 作用：从 Flash 中逐项读取配置，如果该项不存在（如首次开机），则赋予默认值。
// ==========================================
bool settingsLoad() {
    String tempStr; // 用于临时接收 Preferences 读出的字符串

    // --- Wi-Fi 网络配置 ---
    tempStr = prefs.getString("wifiSsid", "");
    strlcpy(globalSettings.wifiSsid, tempStr.c_str(), sizeof(globalSettings.wifiSsid));

    tempStr = prefs.getString("wifiPass", "");
    strlcpy(globalSettings.wifiPass, tempStr.c_str(), sizeof(globalSettings.wifiPass));

    // --- MQTT 物联网配置 ---
    tempStr = prefs.getString("mqttHost", "");
    strlcpy(globalSettings.mqttHost, tempStr.c_str(), sizeof(globalSettings.mqttHost));
    
    globalSettings.mqttPort = prefs.getInt("mqttPort", 1883); // 默认 1883 端口

    tempStr = prefs.getString("mqttUser", "");
    strlcpy(globalSettings.mqttUser, tempStr.c_str(), sizeof(globalSettings.mqttUser));

    tempStr = prefs.getString("mqttPass", "");
    strlcpy(globalSettings.mqttPass, tempStr.c_str(), sizeof(globalSettings.mqttPass));

    // --- AI 助手配置 ---
    tempStr = prefs.getString("aiApi", "http://api.example.com/v1/chat/completions");
    strlcpy(globalSettings.aiApi, tempStr.c_str(), sizeof(globalSettings.aiApi));

    tempStr = prefs.getString("aiKey", "");
    strlcpy(globalSettings.aiKey, tempStr.c_str(), sizeof(globalSettings.aiKey));

    // --- 硬件交互偏好 ---
    globalSettings.brightness = prefs.getInt("brightness", 80); // 默认亮度 80
    globalSettings.volume = prefs.getInt("volume", 50);         // 默认音量 50

    // --- 媒体库：多电台管理 ---
    globalSettings.radioStationCount = prefs.getInt("radioCount", 0);
    // 安全校验：防止因异常导致数量越界
    if (globalSettings.radioStationCount > MAX_RADIO_STATIONS) {
        globalSettings.radioStationCount = MAX_RADIO_STATIONS;
    }

    // [为什么用固定槽位逐项保存？]
    // ESP32 的 Preferences 本质是一个键值对(Key-Value)数据库，不支持直接保存复杂的对象数组。
    // 因此，我们将数组展平。例如保存第 0 个电台的名称，它的 Key 就是 "r_name_0"。
    // 这种做法极其稳定，不需要引入复杂的 JSON 解析库，对初学者最友好。
    char keyBuf[16]; // 存放拼接出来的 Key 名称
    for (int i = 0; i < MAX_RADIO_STATIONS; i++) {
        // [Key 命名策略] 
        // Preferences 强制要求 Key 的长度不能超过 15 个字符。
        // 所以我们用简写：r_name_X, r_url_X, r_en_X
        
        sprintf(keyBuf, "r_name_%d", i);
        // 如果是首次开机(没有配置)，顺手塞一个默认电台进去
        tempStr = prefs.getString(keyBuf, i == 0 ? "Lofi Radio" : ""); 
        strlcpy(globalSettings.radioStations[i].name, tempStr.c_str(), sizeof(globalSettings.radioStations[i].name));

        sprintf(keyBuf, "r_url_%d", i);
        tempStr = prefs.getString(keyBuf, i == 0 ? "http://stream.zeno.fm/f3wvbbqmdg8uv" : "");
        strlcpy(globalSettings.radioStations[i].url, tempStr.c_str(), sizeof(globalSettings.radioStations[i].url));

        sprintf(keyBuf, "r_en_%d", i);
        globalSettings.radioStations[i].enabled = prefs.getBool(keyBuf, i == 0 ? true : false);
    }

    // 针对首次开机的初始化逻辑补丁
    if (globalSettings.radioStationCount == 0 && strlen(globalSettings.radioStations[0].name) > 0) {
        globalSettings.radioStationCount = 1; 
    }

    return true;
}

// ==========================================
// [流程] Save 流程
// 作用：将当前内存(globalSettings)里的所有值，覆盖写回到 Flash 中。
// 注意：应当在 Web UI 用户点击“保存”，或者屏幕调节完亮度松手时调用。
// ==========================================
bool settingsSave() {
    prefs.putString("wifiSsid", globalSettings.wifiSsid);
    prefs.putString("wifiPass", globalSettings.wifiPass);

    prefs.putString("mqttHost", globalSettings.mqttHost);
    prefs.putInt("mqttPort", globalSettings.mqttPort);
    prefs.putString("mqttUser", globalSettings.mqttUser);
    prefs.putString("mqttPass", globalSettings.mqttPass);

    prefs.putString("aiApi", globalSettings.aiApi);
    prefs.putString("aiKey", globalSettings.aiKey);

    prefs.putInt("brightness", globalSettings.brightness);
    prefs.putInt("volume", globalSettings.volume);

    prefs.putInt("radioCount", globalSettings.radioStationCount);

    char keyBuf[16];
    for (int i = 0; i < MAX_RADIO_STATIONS; i++) {
        sprintf(keyBuf, "r_name_%d", i);
        prefs.putString(keyBuf, globalSettings.radioStations[i].name);

        sprintf(keyBuf, "r_url_%d", i);
        prefs.putString(keyBuf, globalSettings.radioStations[i].url);

        sprintf(keyBuf, "r_en_%d", i);
        prefs.putBool(keyBuf, globalSettings.radioStations[i].enabled);
    }

    return true;
}

// ==========================================
// [接口] 初始化并挂载存储
// ==========================================
bool settingsBegin() {
    // 初始化命名空间 "espdesk"，参数 false 代表以读写(Read/Write)模式打开
    bool success = prefs.begin("espdesk", false);
    if (success) {
        // 挂载成功后，立刻读取之前的配置到内存中
        settingsLoad();
    }
    return success;
}