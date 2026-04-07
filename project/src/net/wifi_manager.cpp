#include "wifi_manager.h"
#include "../storage/settings.h"
#include "web_server.h"      // webServerBeginAP / webServerStop / webServerBegin
#include "../../app_data.h"  // SystemState g_systemState
#include "../../wifi_build_config.h"
#include <WiFi.h>
#include <esp_task_wdt.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_wifi.h>

// ==========================================
// 📶 Wi-Fi 网络连接管理器实现 (Wi-Fi Manager)
// ==========================================

// ==========================================
// [接口] 初始化 Wi-Fi 模块
// ==========================================
bool wifiManagerBegin() {
    // 将 ESP32 设置为纯客户端模式 (Station Mode)，不发射自带热点 (AP)
    WiFi.mode(WIFI_STA);
    
    // 断开可能遗留在 NVS 中的旧连接，确保状态干净
    WiFi.disconnect(true); 
    delay(100);
    
    Serial.println("[WiFi] 初始化完成，已切换至 STA 模式");
    return true;
}

// ==========================================
// [接口] 执行 Wi-Fi 连接
// ==========================================
bool wifiManagerConnect() {
    // [读取 settings 中的 WiFi 配置]
    // 假设 settings 模块提供了获取字符串的接口，这里做解耦调用
    String ssid = settingsGetWifiSsid();
    String password = settingsGetWifiPassword();

    // 如果未配置 SSID，直接拦截，避免引发底层库异常
    if (ssid.length() == 0) {
        Serial.println("[WiFi] 错误：未配置 SSID，请先设置网络！");
        return false;
    }

    Serial.printf("[WiFi] 正在尝试连接至: %s\n", ssid.c_str());

    // 启动连接进程
    WiFi.begin(ssid.c_str(), password.c_str());

    // [保持最小可运行版本] 的超时保护机制
    // 每次等待 500ms，最多重试 20 次 (即 10 秒超时)，防止主线程死锁
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();

    // 检查最终状态
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[WiFi] 连接成功！");
        Serial.print("[WiFi] IP 地址: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("[WiFi] 连接超时或密码错误，请检查网络环境。");
        return false;
    }
}

// ==========================================
// [接口] 查询当前 Wi-Fi 实时连接状态
// ==========================================
bool wifiManagerIsConnected() {
    // 严格比对底层状态枚举，返回准确的布尔值
    return WiFi.status() == WL_CONNECTED;
}

// ==========================================
// [接口] 获取本机分配的局域网 IP 地址
// ==========================================
String wifiManagerGetIp() {
    // 只有在确定连接的情况下才去取 IP，否则返回占位符
    if (wifiManagerIsConnected()) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}

// ══════════════════════════════════════════════════════════════════
// WiFi 配网状态机实现
// ══════════════════════════════════════════════════════════════════

extern SystemState g_systemState;  // 定义在 project.ino

// ── 内部状态 ──────────────────────────────────────────────────────
static wifi_state_t s_wifi_state = WIFI_STATE_IDLE;
static bool s_wifi_event_registered = false;
static bool s_raw_ap_wifi_inited = false;
static bool s_raw_ap_events_registered = false;
static esp_netif_t *s_ap_netif = nullptr;

static bool build_config_enabled() {
    return WIFI_BUILD_CONFIG_ENABLED != 0;
}

static bool build_config_has_credentials() {
    return build_config_enabled() && WIFI_BUILD_CONFIG_SSID[0] != '\0';
}

static bool build_config_allow_ap_fallback() {
    return WIFI_BUILD_CONFIG_ALLOW_AP_FALLBACK != 0;
}

static bool build_config_allow_settings_fallback() {
    return WIFI_BUILD_CONFIG_ALLOW_SETTINGS_FALLBACK != 0;
}

static void reset_wifi_state_fields() {
    g_systemState.wifiConnected = false;
    strlcpy(g_systemState.wifiIp, "0.0.0.0", sizeof(g_systemState.wifiIp));
    strlcpy(g_systemState.wifiMask, "0.0.0.0", sizeof(g_systemState.wifiMask));
}

static void start_sta_connection(const char *ssid,
                                 const char *password,
                                 const char *source) {
    if (!s_wifi_event_registered) {
        WiFi.onEvent(onWifiEvent);
        s_wifi_event_registered = true;
    }

    reset_wifi_state_fields();
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(ssid, password);
    s_wifi_state = WIFI_STATE_CONNECTING;
    Serial.printf("[WIFI] %s STA connecting to %s\n", source, ssid);
}

static void onRawApWifiEvent(void *arg,
                             esp_event_base_t event_base,
                             int32_t event_id,
                             void *event_data) {
    (void)arg;

    if (event_base != WIFI_EVENT) return;

    switch (event_id) {
        case WIFI_EVENT_AP_START:
            Serial.println("[WIFI] event: AP_START");
            break;

        case WIFI_EVENT_AP_STOP:
            Serial.println("[WIFI] event: AP_STOP");
            break;

        case WIFI_EVENT_AP_STACONNECTED: {
            wifi_event_ap_staconnected_t *evt =
                static_cast<wifi_event_ap_staconnected_t *>(event_data);
            if (evt) {
                Serial.printf("[WIFI] event: STA_JOIN aid=%u mac=%02X:%02X:%02X:%02X:%02X:%02X\n",
                              evt->aid,
                              evt->mac[0], evt->mac[1], evt->mac[2],
                              evt->mac[3], evt->mac[4], evt->mac[5]);
            }
            break;
        }

        case WIFI_EVENT_AP_STADISCONNECTED: {
            wifi_event_ap_stadisconnected_t *evt =
                static_cast<wifi_event_ap_stadisconnected_t *>(event_data);
            if (evt) {
                Serial.printf("[WIFI] event: STA_LEAVE aid=%u mac=%02X:%02X:%02X:%02X:%02X:%02X\n",
                              evt->aid,
                              evt->mac[0], evt->mac[1], evt->mac[2],
                              evt->mac[3], evt->mac[4], evt->mac[5]);
            }
            break;
        }

        default:
            break;
    }
}

static bool raw_ap_wifi_init_once() {
    if (s_raw_ap_wifi_inited) return true;

    Serial.println("[WIFI] raw init: esp_netif_init");
    esp_err_t err = esp_netif_init();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        Serial.printf("[WIFI] esp_netif_init failed: %d\n", (int)err);
        return false;
    }

    Serial.println("[WIFI] raw init: event loop");
    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        Serial.printf("[WIFI] esp_event_loop_create_default failed: %d\n", (int)err);
        return false;
    }

    if (!s_raw_ap_events_registered) {
        Serial.println("[WIFI] raw init: register AP events");
        err = esp_event_handler_register(WIFI_EVENT,
                                         WIFI_EVENT_AP_START,
                                         &onRawApWifiEvent,
                                         nullptr);
        if (err != ESP_OK) {
            Serial.printf("[WIFI] register AP_START failed: %d\n", (int)err);
            return false;
        }

        err = esp_event_handler_register(WIFI_EVENT,
                                         WIFI_EVENT_AP_STOP,
                                         &onRawApWifiEvent,
                                         nullptr);
        if (err != ESP_OK) {
            Serial.printf("[WIFI] register AP_STOP failed: %d\n", (int)err);
            return false;
        }

        err = esp_event_handler_register(WIFI_EVENT,
                                         WIFI_EVENT_AP_STACONNECTED,
                                         &onRawApWifiEvent,
                                         nullptr);
        if (err != ESP_OK) {
            Serial.printf("[WIFI] register AP_STACONNECTED failed: %d\n", (int)err);
            return false;
        }

        err = esp_event_handler_register(WIFI_EVENT,
                                         WIFI_EVENT_AP_STADISCONNECTED,
                                         &onRawApWifiEvent,
                                         nullptr);
        if (err != ESP_OK) {
            Serial.printf("[WIFI] register AP_STADISCONNECTED failed: %d\n", (int)err);
            return false;
        }

        s_raw_ap_events_registered = true;
    }

    if (!s_ap_netif) {
        Serial.println("[WIFI] raw init: create default AP netif");
        s_ap_netif = esp_netif_create_default_wifi_ap();
        if (!s_ap_netif) {
            Serial.println("[WIFI] esp_netif_create_default_wifi_ap failed");
            return false;
        }
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    Serial.println("[WIFI] raw init: esp_wifi_init");
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK && err != ESP_ERR_WIFI_INIT_STATE) {
        Serial.printf("[WIFI] esp_wifi_init failed: %d\n", (int)err);
        return false;
    }

    Serial.println("[WIFI] raw init: set storage RAM");
    err = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    if (err != ESP_OK) {
        Serial.printf("[WIFI] esp_wifi_set_storage failed: %d\n", (int)err);
        return false;
    }

    s_raw_ap_wifi_inited = true;
    return true;
}

wifi_state_t wifi_get_state(void) {
    return s_wifi_state;
}

bool wifiManagerUsingBuildConfig(void) {
    return build_config_enabled();
}

// ── WiFi 事件回调（ESP32 Arduino 事件系统）─────────────────────────
static void onWifiEvent(WiFiEvent_t event) {
    switch (event) {

        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            // 连接成功，拿到 IP
            s_wifi_state             = WIFI_STATE_CONNECTED;
            g_systemState.wifiConnected = true;
            strncpy(g_systemState.wifiIp,
                    WiFi.localIP().toString().c_str(),
                    sizeof(g_systemState.wifiIp) - 1);
            strncpy(g_systemState.wifiMac,
                    WiFi.macAddress().c_str(),
                    sizeof(g_systemState.wifiMac) - 1);
            Serial.printf("[WIFI] Connected! IP: %s\n", g_systemState.wifiIp);
            // 关闭配网 AP，启动正式 Web UI 服务器
            wifi_stop_ap();
            webServerBegin();
            break;

        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            reset_wifi_state_fields();
            if (build_config_has_credentials() || build_config_allow_settings_fallback()) {
                s_wifi_state = WIFI_STATE_CONNECTING;
                WiFi.reconnect();
                Serial.println("[WIFI] Disconnected, reconnecting...");
            } else {
                s_wifi_state = WIFI_STATE_FAILED;
                Serial.println("[WIFI] Connection failed");
            }
            break;

        default:
            break;
    }
}

// ── 启动 SoftAP 配网热点 ──────────────────────────────────────────
void wifi_start_ap(void) {
    Serial.println("[WIFI] AP step: idf init");
    if (!raw_ap_wifi_init_once()) {
        s_wifi_state = WIFI_STATE_FAILED;
        Serial.println("[WIFI] raw AP init failed");
        return;
    }

    esp_task_wdt_reset();

    wifi_config_t ap_cfg = {};
    memcpy(ap_cfg.ap.ssid, "ESP32_Config", strlen("ESP32_Config"));
    memcpy(ap_cfg.ap.password, "12345678", strlen("12345678"));
    ap_cfg.ap.ssid_len = strlen("ESP32_Config");
    ap_cfg.ap.channel = 1;
    ap_cfg.ap.authmode = WIFI_AUTH_WPA2_PSK;
    ap_cfg.ap.max_connection = 4;
    ap_cfg.ap.ssid_hidden = 0;
    ap_cfg.ap.beacon_interval = 100;

    Serial.println("[WIFI] AP step: set mode");
    esp_err_t err = esp_wifi_set_mode(WIFI_MODE_AP);
    if (err != ESP_OK) {
        s_wifi_state = WIFI_STATE_FAILED;
        Serial.printf("[WIFI] esp_wifi_set_mode failed: %d\n", (int)err);
        return;
    }

    esp_task_wdt_reset();
    delay(20);

    Serial.println("[WIFI] AP step: set config");
    err = esp_wifi_set_config(WIFI_IF_AP, &ap_cfg);
    if (err != ESP_OK) {
        s_wifi_state = WIFI_STATE_FAILED;
        Serial.printf("[WIFI] esp_wifi_set_config failed: %d\n", (int)err);
        return;
    }

    esp_task_wdt_reset();
    delay(20);

    Serial.println("[WIFI] AP step: start");
    err = esp_wifi_start();
    if (err != ESP_OK && err != ESP_ERR_WIFI_CONN) {
        s_wifi_state = WIFI_STATE_FAILED;
        Serial.printf("[WIFI] esp_wifi_start failed: %d\n", (int)err);
        return;
    }

    s_wifi_state = WIFI_STATE_AP;
    esp_netif_ip_info_t ip_info = {};
    if (s_ap_netif) {
        esp_netif_get_ip_info(s_ap_netif, &ip_info);
    }
    IPAddress ap_ip(ip_info.ip.addr);
    Serial.printf("[WIFI] AP started: SSID=ESP32_Config  IP=%u.%u.%u.%u\n",
                  ap_ip[0], ap_ip[1], ap_ip[2], ap_ip[3]);
    webServerBeginAP();   // 启动配网专用 Web 门户
}

// ── 关闭 SoftAP ───────────────────────────────────────────────────
void wifi_stop_ap(void) {
    webServerStop();
    if (s_raw_ap_wifi_inited) {
        esp_err_t err = esp_wifi_stop();
        if (err != ESP_OK && err != ESP_ERR_WIFI_NOT_INIT &&
            err != ESP_ERR_WIFI_NOT_STARTED) {
            Serial.printf("[WIFI] esp_wifi_stop failed: %d\n", (int)err);
        }

        err = esp_wifi_set_mode(WIFI_MODE_NULL);
        if (err != ESP_OK && err != ESP_ERR_WIFI_NOT_INIT) {
            Serial.printf("[WIFI] esp_wifi_set_mode(NULL) failed: %d\n", (int)err);
        }
    } else {
        WiFi.softAPdisconnect(true);
    }
    Serial.println("[WIFI] AP stopped");
}

// ── 自动配网入口（setup() 末尾调用一次）─────────────────────────────
void wifiManagerBeginProvisioning(void) {
    // ESP32-S3 + PSRAM 场景优先用静态 WiFi 缓冲，降低动态分配导致的异常。
    WiFi.useStaticBuffers(true);
    WiFi.persistent(false);
    Serial.println("[WIFI] static buffers ON, persistent OFF");

    if (build_config_has_credentials()) {
        start_sta_connection(WIFI_BUILD_CONFIG_SSID,
                             WIFI_BUILD_CONFIG_PASSWORD,
                             "build-config");
        return;
    }

    if (build_config_enabled()) {
        Serial.println("[WIFI] build-config enabled but SSID is empty");
        s_wifi_state = WIFI_STATE_FAILED;
        return;
    }

    AppSettings &settings = settingsGet();
    if (build_config_allow_settings_fallback() && settings.wifiSsid[0] != '\0') {
        start_sta_connection(settings.wifiSsid,
                             settings.wifiPass,
                             "settings");
        return;
    }

    if (build_config_allow_ap_fallback()) {
        Serial.println("[WIFI] No static SSID, starting AP provisioning");
        wifi_start_ap();
        return;
    }

    Serial.println("[WIFI] No WiFi credentials and AP fallback disabled");
    s_wifi_state = WIFI_STATE_FAILED;
}

// ── 状态机轮询（loop() 中调用）────────────────────────────────────
void wifiManagerLoop(void) {
    // Web Server 需要在 loop 中持续处理客户端请求
    if (s_wifi_state == WIFI_STATE_AP ||
        s_wifi_state == WIFI_STATE_CONNECTED) {
        webServerLoop();
    }
    // CONNECTING 超时保护：60s 后若仍未连接则回退到 AP 模式
    static unsigned long connectStart = 0;
    if (s_wifi_state == WIFI_STATE_CONNECTING) {
        if (connectStart == 0) connectStart = millis();
        if (millis() - connectStart > 60000UL) {
            connectStart = 0;
            if (build_config_allow_ap_fallback() && !build_config_has_credentials()) {
                Serial.println("[WIFI] Timeout, falling back to AP");
                wifi_start_ap();
            } else {
                s_wifi_state = WIFI_STATE_FAILED;
                WiFi.disconnect(false, false);
                Serial.println("[WIFI] Timeout, static STA failed");
            }
        }
    } else {
        connectStart = 0;
    }
}