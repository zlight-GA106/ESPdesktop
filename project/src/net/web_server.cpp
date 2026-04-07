#include "web_server.h"
#include "../storage/settings.h"
#include "../../app_data.h"
#include <WebServer.h>

// ==========================================
// 🌐 本地 Web 服务器实现 (Web Server)
// ==========================================

// 实例化 WebServer 对象，监听标准 HTTP 80 端口
WebServer server(80);
static bool s_reboot_requested = false;
static unsigned long s_reboot_due = 0;

// 声明外部系统状态变量，以便在网页上实时显示传感器与系统状态
// (确保 app_data.h 和主程序中有对应的全局变量定义)
extern SensorData g_sensorData;
extern SystemState g_systemState;

static void scheduleReboot(unsigned long delayMs) {
    s_reboot_requested = true;
    s_reboot_due = millis() + delayMs;
}

bool webServerShouldReboot(void) {
    if (!s_reboot_requested) return false;

    long diff = (long)(millis() - s_reboot_due);
    if (diff >= 0) {
        s_reboot_requested = false;
        s_reboot_due = 0;
        return true;
    }
    return false;
}

// ==========================================
// [内部辅助函数] 生成通用的 HTML 头部和导航栏
// ==========================================
String getHtmlHeader(String title) {
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>" + title + " - ESP Desktop</title>";
    html += "<style>";
    html += "body { font-family: sans-serif; margin: 20px; line-height: 1.6; }";
    html += "table { border-collapse: collapse; width: 100%; max-width: 600px; margin-bottom: 20px; }";
    html += "th, td { border: 1px solid #ccc; padding: 8px; text-align: left; }";
    html += "th { background-color: #f4f4f4; }";
    html += "input[type='text'], input[type='password'] { width: 100%; padding: 8px; box-sizing: border-box; }";
    html += "button, input[type='submit'] { padding: 10px 15px; background: #000080; color: #fff; border: none; cursor: pointer; }";
    html += "button:hover, input[type='submit']:hover { background: #0000cd; }";
    html += ".nav { margin-bottom: 20px; padding-bottom: 10px; border-bottom: 2px solid #eee; }";
    html += ".nav a { margin-right: 15px; text-decoration: none; color: #000080; font-weight: bold; }";
    html += "</style></head><body>";
    
    // 公共导航栏
    html += "<div class='nav'>";
    html += "<a href='/'>📊 状态总览</a>";
    html += "<a href='/config'>⚙️ 基础配置</a>";
    html += "<a href='/radio'>📻 电台管理</a>";
    html += "</div>";
    
    html += "<h2>" + title + "</h2>";
    return html;
}

String getHtmlFooter() {
    return "</body></html>";
}

// ==========================================
// [路由处理] 1. 根目录 / (状态页)
// ==========================================
void handleRoot() {
    String html = getHtmlHeader("系统运行状态");
    
    html += "<table>";
    html += "<tr><th>参数</th><th>当前值</th></tr>";
    
    // [1] 传感器状态
    html += "<tr><td>🌡️ 温度 (Temperature)</td><td>" + String(g_sensorData.temperature, 1) + " °C</td></tr>";
    html += "<tr><td>💧 湿度 (Humidity)</td><td>" + String(g_sensorData.humidity, 1) + " %</td></tr>";
    html += "<tr><td>☀️ 光照 (Light)</td><td>" + String(g_sensorData.lightLevel) + " Lux</td></tr>";
    html += "<tr><td>🚶 人体状态 (PIR)</td><td>" + String(g_sensorData.humanDetected ? "检测到移动" : "静止") + "</td></tr>";
    html += "<tr><td>🔊 声音强度 (Mic)</td><td>" + String(g_sensorData.soundLevel) + "</td></tr>";
    
    // [2] 系统运行状态
    html += "<tr><td>📶 WiFi 状态</td><td>" + String(g_systemState.wifiConnected ? "已连接" : "未连接") + " (" + String(g_systemState.wifiIp) + ")</td></tr>";
    html += "<tr><td>☁️ MQTT 状态</td><td>" + String(g_systemState.mqttConnected ? "已连接" : "未连接") + "</td></tr>";
    // 假设 aiState: 0=待机, 1=请求中, 2=播报中
    String aiStateStr = "待机";
    if (g_systemState.aiState == 1) aiStateStr = "思考中...";
    if (g_systemState.aiState == 2) aiStateStr = "播报中";
    html += "<tr><td>🤖 AI 状态</td><td>" + aiStateStr + "</td></tr>";
    
    html += "</table>";
    html += "<button onclick='location.reload()'>刷新状态</button>";
    html += getHtmlFooter();
    
    server.send(200, "text/html", html);
}

// ==========================================
// [路由处理] 2. 基础配置页 /config
// ==========================================
void handleConfig() {
    String html = getHtmlHeader("基础参数配置");
    
    // 表单提交至 /save_config
    html += "<form action='/save_config' method='POST'>";
    html += "<table>";
    
    html += "<tr><th colspan='2'>WiFi 设置</th></tr>";
    html += "<tr><td>SSID:</td><td><input type='text' name='wifi_ssid' value='" + settingsGetWifiSsid() + "'></td></tr>";
    // 密码不直接回显明文，若已设置则显示提示符
    html += "<tr><td>Password:</td><td><input type='password' name='wifi_pwd' placeholder='留空则不修改'></td></tr>";
    
    html += "<tr><th colspan='2'>MQTT 设置</th></tr>";
    html += "<tr><td>Broker URL:</td><td><input type='text' name='mqtt_broker' value='" + settingsGetMqttBroker() + "'></td></tr>";
    
    html += "<tr><th colspan='2'>AI 接口设置</th></tr>";
    html += "<tr><td>API URL/Key:</td><td><input type='text' name='ai_api' value='" + settingsGetAiApi() + "'></td></tr>";
    
    html += "</table>";
    html += "<input type='submit' value='💾 保存并重启'>";
    html += "</form>";
    html += getHtmlFooter();
    
    server.send(200, "text/html", html);
}

// ==========================================
// [路由处理] 3. 保存配置 /save_config
// ==========================================
void handleSaveConfig() {
    if (server.method() != HTTP_POST) {
        server.send(405, "text/plain", "Method Not Allowed");
        return;
    }

    // 逐一读取表单参数并存入 settings
    if (server.hasArg("wifi_ssid")) {
        String ssid = server.arg("wifi_ssid");
        String pwd = server.arg("wifi_pwd");
        if (ssid.length() > 0) {
            settingsSetWifiSsid(ssid);
            if (pwd.length() > 0) settingsSetWifiPassword(pwd); // 仅当输入密码时才覆盖
        }
    }
    
    if (server.hasArg("mqtt_broker")) {
        settingsSetMqttBroker(server.arg("mqtt_broker"));
    }
    
    if (server.hasArg("ai_api")) {
        settingsSetAiApi(server.arg("ai_api"));
    }

    String html = getHtmlHeader("保存成功");
    html += "<p>配置已成功写入 Flash！系统将在几秒后重启以应用新配置。</p>";
    html += "<p><a href='/config'>返回配置页</a></p>";
    html += getHtmlFooter();
    
    server.send(200, "text/html", html);
    scheduleReboot(1000);
    Serial.println("[WEB] Reboot scheduled after /save_config");
}

// ==========================================
// [路由处理] 4. 电台管理页 /radio
// ==========================================
void handleRadio() {
    String html = getHtmlHeader("网络电台管理");
    
    // 当前电台列表
    html += "<h3>当前预设列表</h3>";
    html += "<table><tr><th>#</th><th>电台名称</th><th>媒体流 URL</th><th>操作</th></tr>";
    
    int count = settingsGetRadioCount();
    for (int i = 0; i < count; i++) {
        html += "<tr>";
        html += "<td>" + String(i + 1) + "</td>";
        html += "<td>" + settingsGetRadioName(i) + "</td>";
        // 限制长 URL 的显示长度，避免撑破表格
        html += "<td style='word-break: break-all;'>" + settingsGetRadioUrl(i) + "</td>";
        html += "<td><a href='/del_radio?id=" + String(i) + "' onclick='return confirm(\"确定删除吗?\");'>❌ 删除</a></td>";
        html += "</tr>";
    }
    if (count == 0) {
        html += "<tr><td colspan='4'>暂无电台，请在下方添加。</td></tr>";
    }
    html += "</table>";
    
    // 添加电台的表单
    html += "<h3>添加新电台</h3>";
    html += "<form action='/add_radio' method='POST'>";
    html += "<table>";
    html += "<tr><td>名称 (Name):</td><td><input type='text' name='radio_name' required></td></tr>";
    html += "<tr><td>流地址 (URL):</td><td><input type='text' name='radio_url' required placeholder='http://...'></td></tr>";
    html += "</table>";
    html += "<input type='submit' value='➕ 添加电台'>";
    html += "</form>";
    
    html += getHtmlFooter();
    server.send(200, "text/html", html);
}

// ==========================================
// [路由处理] 5. 添加电台逻辑 /add_radio
// ==========================================
void handleAddRadio() {
    if (server.hasArg("radio_name") && server.hasArg("radio_url")) {
        String name = server.arg("radio_name");
        String url = server.arg("radio_url");
        settingsAddRadio(name, url);
    }
    // 添加完成后重定向回 /radio 页面
    server.sendHeader("Location", "/radio");
    server.send(303);
}

// ==========================================
// [路由处理] 6. 删除电台逻辑 /del_radio
// ==========================================
void handleDelRadio() {
    if (server.hasArg("id")) {
        int id = server.arg("id").toInt();
        settingsDeleteRadio(id);
    }
    // 删除完成后重定向回 /radio 页面
    server.sendHeader("Location", "/radio");
    server.send(303);
}

// ==========================================
// [接口] 初始化并启动 Web 服务器
// ==========================================
bool webServerBegin() {
    // 注册 URL 路由端点与回调函数的映射关系
    server.on("/", HTTP_GET, handleRoot);
    server.on("/config", HTTP_GET, handleConfig);
    server.on("/save_config", HTTP_POST, handleSaveConfig);
    server.on("/radio", HTTP_GET, handleRadio);
    server.on("/add_radio", HTTP_POST, handleAddRadio);
    server.on("/del_radio", HTTP_GET, handleDelRadio);
    
    // 处理未找到页面的 404 错误
    server.onNotFound([]() {
        server.send(404, "text/plain", "404: Not Found");
    });
    
    // 启动服务器
    server.begin();
    Serial.println("[Web] 本地 HTTP 服务器已启动，端口: 80");
    return true;
}

// ==========================================
// [接口] Web 服务器请求处理泵 (Tick)
// ==========================================
void webServerLoop() {
    // 接收并处理客户端的 HTTP 请求。必须在 main.cpp 的 loop() 中持续调用。
    server.handleClient();
}

// ══════════════════════════════════════════════════════════════════
// SoftAP 配网专用 Web 服务器
//
// 设计原则：AP 配网提交后执行 ESP.restart()
//   ① 彻底避免 AP/STA 双模式下路由表污染
//   ② 重启后 NVS 已有凭据 → 自动进入 STA 模式连接
//   ③ 连接成功后 webServerBegin() 启动正式生产服务器
// ══════════════════════════════════════════════════════════════════

// ──── HTML 常量（PROGMEM，节省运行时 SRAM）────────────────────────

static const char AP_PORTAL_HTML[] PROGMEM =
    "<!DOCTYPE html><html><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>WiFi Setup</title>"
    "<style>"
    "*{box-sizing:border-box}"
    "body{font-family:sans-serif;background:#1a1a2e;color:#e0e0e0;"
         "display:flex;justify-content:center;align-items:center;"
         "min-height:100vh;margin:0;padding:12px}"
    ".card{background:#0d3b6e;padding:28px 22px;border-radius:12px;"
           "width:100%;max-width:360px}"
    "h2{text-align:center;margin:0 0 22px 0;font-size:19px}"
    "label{display:block;font-size:13px;color:#90caf9;margin-bottom:4px}"
    "input{width:100%;padding:10px 9px;margin-bottom:15px;"
          "border:1px solid #1e4d8c;border-radius:6px;"
          "background:#162040;color:#eee;font-size:15px}"
    "input:focus{outline:none;border-color:#42a5f5}"
    "button{width:100%;padding:13px;background:#1565c0;color:#fff;"
           "border:none;border-radius:6px;font-size:16px;cursor:pointer;margin-top:2px}"
    "button:active{background:#0d47a1}"
    ".hint{text-align:center;margin-top:14px;font-size:12px;color:#7799bb;line-height:1.5}"
    "</style></head>"
    "<body><div class='card'>"
    "<h2>&#128246; WiFi &#37197;&#32593;</h2>"
    "<form method='POST' action='/connect'>"
    "<label>WiFi &#21517;&#31216; (SSID)</label>"
    "<input type='text' name='ssid' placeholder='&#35831;&#36755;&#20837; WiFi &#21517;&#31216;' required autocomplete='off'>"
    "<label>&#23494;&#30721; (Password)</label>"
    "<input type='password' name='password' placeholder='&#26080;&#23494;&#30721;&#35831;&#30041;&#31354;' autocomplete='off'>"
    "<button type='submit'>&#9889; &#36830;&#25509;</button>"
    "</form>"
    "<p class='hint'>&#128225; &#36830;&#25509;&#30028;&#38754;&#65306;ESP32_Config<br>"
    "&#128279; 192.168.4.1 (&#26412;&#39029;)</p>"
    "</div></body></html>";

static const char AP_SAVING_HTML[] PROGMEM =
    "<!DOCTYPE html><html><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>&#24050;&#20445;&#23384;</title>"
    "<style>"
    "body{font-family:sans-serif;background:#1a2e1a;color:#e0e0e0;"
         "display:flex;justify-content:center;align-items:center;"
         "min-height:100vh;margin:0;padding:12px}"
    ".card{background:#1b3a1b;padding:28px 22px;border-radius:12px;"
           "width:100%;max-width:360px;text-align:center}"
    "h2{color:#66bb6a;margin:0 0 16px 0}"
    "p{color:#aaccaa;font-size:14px;line-height:1.7;margin:8px 0}"
    ".ssid{color:#81d4fa;font-weight:bold}"
    "</style></head>"
    "<body><div class='card'>"
    "<h2>&#10003; &#20381;&#25454;&#24050;&#20445;&#23384;</h2>"
    "<p>&#35774;&#22791;&#27491;&#22312;&#37325;&#21551;&#24182;&#23581;&#35797;&#36830;&#25509; WiFi&hellip;</p>"
    "<p>&#36830;&#25509;&#25104;&#21151;&#21518;&#65292;<b>ESP32_Config</b> &#30070;&#28857;&#23558;&#28040;&#22833;&#12290;</p>"
    "<p>&#35831;&#23558;&#25163;&#26426;&#20851;&#32852;&#22238;&#24847;&#23478;&#20977; WiFi&#12290;</p>"
    "</div></body></html>";

static const char AP_ERROR_HTML[] PROGMEM =
    "<!DOCTYPE html><html><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>&#38169;&#35823;</title>"
    "<style>"
    "body{font-family:sans-serif;background:#2e1a1a;color:#e0e0e0;"
         "display:flex;justify-content:center;align-items:center;"
         "min-height:100vh;margin:0;padding:12px}"
    ".card{background:#3a1b1b;padding:28px;border-radius:12px;"
           "width:100%;max-width:360px;text-align:center}"
    "h2{color:#ef5350;margin:0 0 14px 0}"
    "a{color:#90caf9;text-decoration:none}"
    "</style></head>"
    "<body><div class='card'>"
    "<h2>&#10060; SSID &#19981;&#33021;&#20026;&#31354;</h2>"
    "<p><a href='/'>&#8592; &#36820;&#22238;&#37325;&#22635;</a></p>"
    "</div></body></html>";

// ──── AP 路由处理函数 ─────────────────────────────────────────────

static void handleApRoot() {
    server.send_P(200, "text/html; charset=utf-8", AP_PORTAL_HTML);
}

static void handleApConnect() {
    if (server.method() != HTTP_POST) {
        server.sendHeader("Location", "/");
        server.send(302, "text/plain", "Redirect");
        return;
    }

    String ssid     = server.arg("ssid");
    String password = server.arg("password");
    ssid.trim();

    if (ssid.length() == 0) {
        server.send_P(400, "text/html; charset=utf-8", AP_ERROR_HTML);
        return;
    }

    Serial.printf("[WEB-AP] Received ssid='%s' pass_len=%d\n",
                  ssid.c_str(), password.length());

    // 存入 NVS
    settingsSetWifiSsid(ssid);
    settingsSetWifiPassword(password);
    settingsSave();
    Serial.println("[WEB-AP] Credentials saved to NVS");

    // 先将 HTML 推给浏览器（flush），再重启，避免 TCP 半开
    server.send_P(200, "text/html; charset=utf-8", AP_SAVING_HTML);
    scheduleReboot(800);
    Serial.println("[WEB-AP] Reboot scheduled to connect STA...");
}

// Captive Portal 捕获：所有未注册路径全部重定向到门户页
static void handleApCaptive() {
    server.sendHeader("Location", "http://192.168.4.1/");
    server.send(302, "text/plain", "Redirect to captive portal");
}

// ──── 公开接口实现 ────────────────────────────────────────────────

/**
 * @brief 启动 SoftAP 配网专用 HTTP 服务器
 *        注册两条核心路由 + Captive Portal 捕获路由
 *        提交后执行 ESP.restart()，下次启动自动走 STA 模式
 */
bool webServerBeginAP(void) {
    // 两条核心路由
    server.on("/",       HTTP_GET,  handleApRoot);
    server.on("/connect", HTTP_POST, handleApConnect);

    // Captive Portal 辅助路由（自动弹出配网页）
    //   Android: /generate_204
    //   iOS    : /hotspot-detect.html
    //   Windows: /ncsi.txt  /connecttest.txt
    server.on("/generate_204",           HTTP_GET, handleApRoot);
    server.on("/hotspot-detect.html",    HTTP_GET, handleApRoot);
    server.on("/ncsi.txt",               HTTP_GET, handleApRoot);
    server.on("/connecttest.txt",        HTTP_GET, handleApRoot);

    // 其余所有未知 URL 重定向到门户页
    server.onNotFound(handleApCaptive);

    server.begin();
    Serial.println("[WEB-AP] Captive portal ready: http://192.168.4.1/");
    Serial.println("[WEB-AP] SSID: ESP32_Config  PASS: 12345678");
    return true;
}

/**
 * @brief 停止 HTTP 服务器（连接成功后、切换到正式服务器前调用）
 */
void webServerStop(void) {
    server.stop();
    Serial.println("[WEB] HTTP server stopped");
}
