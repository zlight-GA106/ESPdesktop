#pragma once
#include <Arduino.h>

// ==========================================
// 📶 Wi-Fi 网络连接管理器接口 (Wi-Fi Manager)
// 职责：负责 ESP32 的无线网络初始化、连接动作与状态查询
// ==========================================

// 初始化 Wi-Fi 模块
// [作用]：将 ESP32 的无线射频模块设置为 Station (客户端) 模式，
// 并断开任何可能遗留的旧连接，为全新的网络连接做好准备。
// [返回值]：初始化成功返回 true，发生硬件级异常返回 false。
bool wifiManagerBegin();

// 执行 Wi-Fi 连接
// [作用]：读取系统配置中的 SSID 和密码，并向路由器发起握手请求。
// 这个过程通常需要耗费几秒钟时间，代码内部会进行非阻塞或带超时的轮询等待。
// [返回值]：成功获取到 IP 地址并连上路由器返回 true，超时或密码错误返回 false。
bool wifiManagerConnect();

// 查询当前 Wi-Fi 实时连接状态
// [作用]：供主循环 (Main Loop) 或 UI 状态栏高频调用，
// 实时探测底层协议栈是否掉线（如路由器重启或信号丢失）。
// [返回值]：当前处于已连接状态返回 true，掉线或未连接返回 false。
bool wifiManagerIsConnected();

// 获取本机分配的局域网 IP 地址
// [作用]：将底层的 IPAddress 对象转换为人类可读的字符串，
// 方便直接输送给 UI 层的 wifi_info_page 进行展示。
// [返回值]：返回类似 "192.168.1.100" 的字符串；若未连接则返回 "0.0.0.0"。
String wifiManagerGetIp();

// ══════════════════════════════════════════════════════════════════
// WiFi 配网状态机（SoftAP + Web Portal 自动配网）
// ══════════════════════════════════════════════════════════════════

/**
 * @brief WiFi 工作状态枚举
 */
typedef enum {
    WIFI_STATE_IDLE       = 0,  ///< 未初始化 / 待机
    WIFI_STATE_AP         = 1,  ///< SoftAP 配网热点运行中（等待用户填写）
    WIFI_STATE_CONNECTING = 2,  ///< 正在尝试连接 WiFi
    WIFI_STATE_CONNECTED  = 3,  ///< 已连接，拿到 IP
    WIFI_STATE_FAILED     = 4,  ///< 连接失败（密码错误 / 超时）
} wifi_state_t;

/**
 * @brief 获取当前 WiFi 状态机状态
 */
wifi_state_t wifi_get_state(void);

/**
 * @brief 自动配网入口（在 setup() 末尾调用一次）
 *        - 若 NVS 中存有 SSID → 直接 STA 模式连接
 *        - 若无配置         → 启动 SoftAP + Web 配网门户
 */
void wifiManagerBeginProvisioning(void);

/**
 * @brief 启动 SoftAP 配网热点
 *        SSID: "ESP32_Config"  密码: "12345678"  IP: 192.168.4.1
 */
void wifi_start_ap(void);

/**
 * @brief 关闭 SoftAP 并停止配网 Web 服务器
 */
void wifi_stop_ap(void);

/**
 * @brief 状态机 / Web 服务器轮询驱动，必须放在 loop() 中调用
 */
void wifiManagerLoop(void);

/**
 * @brief 当前是否启用了编译期静态 WiFi 配置
 */
bool wifiManagerUsingBuildConfig(void);