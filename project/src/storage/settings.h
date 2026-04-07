#pragma once
#include <Arduino.h>

// ==========================================
// 💾 本地持久化配置模块 (存储层)
// ==========================================

// 定义最大支持的网络电台数量
const int MAX_RADIO_STATIONS = 8;

// ==========================================
// 📻 网络电台源结构体
// [为什么要有 RadioStation？]
// 因为项目需要支持多个电台频道的切换，而不仅仅是一个固定的电台。
// 将名称、URL 和启用状态打包成一个整体，UI 模块 (RadioPage) 就能很方便地
// 遍历这个结构体数组，在屏幕上渲染出电台列表。
// ==========================================
struct RadioStation {
    char name[32];   // 电台显示名称 (如: "CCTV News", "Lofi Girl")
    char url[192];   // 电台音频流地址 (如: "http://xxx.mp3")
    bool enabled;    // 是否启用该电台 (方便在 Web UI 中临时隐藏某个失效电台)
};

// ==========================================
// ⚙️ 全局应用设置结构体
// [为什么用固定长度字符数组 (char[]) 而不是 String 类？]
// 1. 内存防爆：String 对象在频繁修改时极易产生内存碎片(Heap Fragmentation)，导致死机。
// 2. 存储高效：固定大小的 struct 内存是连续的，可以直接用单条语句把整个结构体写入 
//    EEPROM 或 Preferences，读写极其快速且对初学者最友好。
// ==========================================
struct AppSettings {
    // --- Wi-Fi 网络配置 ---
    char wifiSsid[32];
    char wifiPass[64];

    // --- MQTT 物联网配置 (对接 Home Assistant) ---
    char mqttHost[64];
    int mqttPort;
    char mqttUser[32];
    char mqttPass[64];

    // --- AI 助手配置 ---
    char aiApi[128];     // AI 接口的基准 URL
    char aiKey[128];     // AI 接口的 API Key

    // --- 媒体库：多电台管理 ---
    RadioStation radioStations[MAX_RADIO_STATIONS];
    
    // [为什么要有 radioStationCount？]
    // 数组的物理大小是固定的(8个)，但用户实际上可能只保存了 3 个电台。
    // 这个变量用于记录当前真正有效的电台数量。UI 渲染列表和循环播放时，
    // 只遍历到 count 的位置，防止读到后面的乱码空数据。
    int radioStationCount;

    // --- 硬件交互偏好 ---
    int brightness;      // 默认屏幕亮度 (0-100)
    int volume;          // 默认系统音量 (0-100)
};

// ==========================================
// 🛠️ Settings 模块对外操作接口
// ==========================================

// 初始化存储模块，必须在 project.ino 的 setup() 最早阶段调用
bool settingsBegin();

// 从 Flash 中读取之前保存的配置到内存
bool settingsLoad();

// 将当前内存中的配置状态一次性安全保存到 Flash
bool settingsSave();

// 获取全局配置数据的引用
// 其他模块（如 Web UI、音频模块）通过调用此接口来读取或修改配置项
AppSettings& settingsGet();

// --- 便捷访问接口 (Getter/Setter) ---
// 为了兼容 web_server.cpp 的调用方式，提供快速 getter
inline String settingsGetWifiSsid() {
    return String(settingsGet().wifiSsid);
}
inline String settingsGetWifiPassword() {
    return String(settingsGet().wifiPass);
}
inline String settingsGetMqttBroker() {
    return String(settingsGet().mqttHost);
}
inline String settingsGetAiApi() {
    return String(settingsGet().aiApi);
}

// Setter
inline void settingsSetWifiSsid(const String &ssid) {
    strlcpy(settingsGet().wifiSsid, ssid.c_str(), sizeof(settingsGet().wifiSsid));
}
inline void settingsSetWifiPassword(const String &pwd) {
    strlcpy(settingsGet().wifiPass, pwd.c_str(), sizeof(settingsGet().wifiPass));
}
inline void settingsSetMqttBroker(const String &broker) {
    strlcpy(settingsGet().mqttHost, broker.c_str(), sizeof(settingsGet().mqttHost));
}
inline void settingsSetAiApi(const String &api) {
    strlcpy(settingsGet().aiApi, api.c_str(), sizeof(settingsGet().aiApi));
}

// 电台管理接口
inline int settingsGetRadioCount() {
    return settingsGet().radioStationCount;
}
inline String settingsGetRadioName(int idx) {
    if (idx >= 0 && idx < MAX_RADIO_STATIONS) {
        return String(settingsGet().radioStations[idx].name);
    }
    return "";
}
inline String settingsGetRadioUrl(int idx) {
    if (idx >= 0 && idx < MAX_RADIO_STATIONS) {
        return String(settingsGet().radioStations[idx].url);
    }
    return "";
}
inline void settingsAddRadio(const String &name, const String &url) {
    AppSettings &s = settingsGet();
    if (s.radioStationCount < MAX_RADIO_STATIONS) {
        strlcpy(s.radioStations[s.radioStationCount].name, name.c_str(), 
                sizeof(s.radioStations[s.radioStationCount].name));
        strlcpy(s.radioStations[s.radioStationCount].url, url.c_str(), 
                sizeof(s.radioStations[s.radioStationCount].url));
        s.radioStations[s.radioStationCount].enabled = true;
        s.radioStationCount++;
        settingsSave();
    }
}
inline void settingsDeleteRadio(int idx) {
    AppSettings &s = settingsGet();
    if (idx >= 0 && idx < s.radioStationCount) {
        for (int i = idx; i < s.radioStationCount - 1; i++) {
            s.radioStations[i] = s.radioStations[i + 1];
        }
        s.radioStationCount--;
        settingsSave();
    }
}