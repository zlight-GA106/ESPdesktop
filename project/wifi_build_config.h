#pragma once

// ==========================================
// WiFi 编译期静态配置
//
// 用法：
// 1. 把下面的 SSID / PASSWORD 改成你家里的 WiFi。
// 2. 重新编译烧录。
// 3. 默认不再启用 AP 配网回退，避免启动链路过重。
// ==========================================

#define WIFI_BUILD_CONFIG_ENABLED           1
#define WIFI_BUILD_CONFIG_ALLOW_AP_FALLBACK 0
#define WIFI_BUILD_CONFIG_ALLOW_SETTINGS_FALLBACK 0

#define WIFI_BUILD_CONFIG_SSID              ""
#define WIFI_BUILD_CONFIG_PASSWORD          ""