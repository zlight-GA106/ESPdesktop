#pragma once

// ==========================================
// 🌐 本地 Web 服务器接口 (Web Server)
// 职责：提供网页配网、参数配置和状态查询的 HTTP 服务
// ==========================================

// 初始化并启动 Web 服务器
// [作用]：在默认的 80 端口上启动 HTTP 监听，并注册对应的 URL 路由端点 (Endpoints)，
// 例如 "/" 对应主页，"/save" 对应保存配置。
// 注意：必须在 Wi-Fi 成功连接，或者开启了 AP (热点) 模式之后才能调用此函数。
// [返回值]：服务器启动成功返回 true，端口被占用或失败返回 false。
bool webServerBegin();

// Web 服务器请求处理泵 (Tick)
// [作用]：需要放置在全局主循环 (Main Loop) 中高频轮询调用。
// 它负责实时侦听是否有客户端 (浏览器) 发起了 HTTP 请求并进行响应分发。
// 注意：如果你底层决定采用 ESPAsyncWebServer (异步服务器)，这个函数内部可以留空，
// 但为了保持架构接口的一致性和对原生 WebServer 的兼容，我们强制要求保留此声明。
void webServerLoop();

// ══════════════════════════════════════════════════════════════════
// SoftAP 配网专用 Web 服务器接口
// ══════════════════════════════════════════════════════════════════

/**
 * @brief 启动 AP 配网专用 HTTP 服务器
 *        仅注册两个路由：
 *        GET  /         → 返回 WiFi 配置表单 HTML
 *        POST /connect  → 解析 ssid/password，保存 NVS，发起连接
 *        未知路径全部重定向到 "/"（Captive Portal 效果）
 * @return true 启动成功
 */
bool webServerBeginAP(void);

/**
 * @brief 停止并关闭 HTTP 服务器（切换模式前调用）
 */
void webServerStop(void);

/**
 * @brief 查询是否有延迟重启请求已到期
 */
bool webServerShouldReboot(void);