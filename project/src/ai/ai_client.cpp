#include "ai_client.h"
#include "../storage/settings.h"
#include "../audio/audio_player.h"

// 引入 HTTP 和网络库
#include <WiFi.h>
#include <HTTPClient.h>

// 引入 FreeRTOS 任务管理
#include <freertos/task.h>

// 明确引用的第三方库：ArduinoJson (作者: Benoit Blanchon)
// 用于解析 AI 返回的复杂 JSON 数据，请在库管理器中安装 (推荐 v6 或 v7 版本)
#include <ArduinoJson.h>

// ==========================================
// 🤖 AI 客户端实现 (AI Client)
// ==========================================

// 内部状态与全局变量
static volatile bool g_aiBusy = false; // 标记 AI 是否正在处理网络请求
static String g_lastReply = "等待唤醒...";   // 存储最后一次 AI 的回复文本
static String g_pendingQuestion = "";  // 暂存用户准备发送的提问

// ==========================================
// [内部辅助函数] 异步 HTTP 请求任务 (FreeRTOS Task)
// 作用：将几秒钟的 HTTP 阻塞操作剥离出主循环，防止屏幕卡死
// ==========================================
void aiHttpRequestTask(void *pvParameters) {
    g_aiBusy = true;

    // 1. 获取 API 配置
    String apiUrl = settingsGetAiApi(); // 例如: "http://192.168.1.100:5000/chat"
    // 注意：如果你的 API 需要 Authorization Bearer Token，也可以从 settings 中读取 Key
    // String apiKey = settingsGetAiKey(); 

    if (apiUrl.length() == 0 || WiFi.status() != WL_CONNECTED) {
        g_lastReply = "错误：网络未连接或未配置 AI API 地址。";
        g_aiBusy = false;
        vTaskDelete(NULL); // 销毁当前任务
        return;
    }

    Serial.printf("[AI] 发起请求 -> %s\n", apiUrl.c_str());

    // 2. 组装 HTTP 客户端与 Payload
    HTTPClient http;
    http.begin(apiUrl);
    http.addHeader("Content-Type", "application/json");

    // 构建极简的 JSON 请求体 (假设服务端接收 {"text": "用户的提问"})
    // 实际项目中可根据 OpenAI 等官方 API 格式调整
    String payload = "{\"text\":\"" + g_pendingQuestion + "\"}";

    // 3. 发起 POST 请求 (此处会阻塞当前 Task，但不会阻塞主循环)
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
        String responseStr = http.getString();
        Serial.printf("[AI] 收到回复 (Code: %d): %s\n", httpResponseCode, responseStr.c_str());

        // 4. 解析 JSON 返回值
        // 假设服务端返回格式: {"reply": "你好！", "audio_url": "http://.../tts.mp3"}
        StaticJsonDocument<1024> doc; 
        DeserializationError error = deserializeJson(doc, responseStr);

        if (!error) {
            // 提取回复文本
            const char* replyText = doc["reply"];
            if (replyText) {
                g_lastReply = String(replyText);
            } else {
                g_lastReply = "AI 返回了无法识别的格式。";
            }

            // [核心联动] 提取音频 URL 并交由 audio_player 播放
            const char* audioUrl = doc["audio_url"];
            if (audioUrl && strlen(audioUrl) > 5) {
                Serial.printf("[AI] 发现 TTS 音频链接，触发底播放: %s\n", audioUrl);
                audioPlayerPlayUrl(String(audioUrl));
            }
        } else {
            g_lastReply = "JSON 解析失败";
            Serial.println("[AI] JSON 解析错误");
        }
    } else {
        g_lastReply = "HTTP 请求失败: " + String(http.errorToString(httpResponseCode).c_str());
        Serial.printf("[AI] %s\n", g_lastReply.c_str());
    }

    // 5. 收尾工作
    http.end();
    g_aiBusy = false;

    // 自杀以释放当前 FreeRTOS 任务所占用的内存 (Stack)
    vTaskDelete(NULL);
}

// ==========================================
// [接口] 初始化 AI 客户端
// ==========================================
bool aiBegin() {
    g_aiBusy = false;
    Serial.println("[AI] 客户端模块初始化完成");
    return true;
}

// ==========================================
// [接口] 发起 AI 请求 (非阻塞设计)
// ==========================================
bool aiAsk(const String &text) {
    if (g_aiBusy) {
        Serial.println("[AI] 系统正忙，拒绝新的请求");
        return false;
    }

    g_pendingQuestion = text;
    g_lastReply = "思考中 (Thinking)...";

    // 创建 FreeRTOS 任务：任务函数, 任务名称, 栈大小(字节), 传参, 优先级, 句柄
    // 分配 8KB (8192) 的栈空间，以防 HTTPS 握手或 JSON 解析时发生栈溢出崩溃
    xTaskCreate(aiHttpRequestTask, "AI_Task", 8192, NULL, 1, NULL);

    return true;
}

// ==========================================
// [接口] 查询 AI 客户端是否正忙
// ==========================================
bool aiIsBusy() {
    return g_aiBusy;
}

// ==========================================
// [接口] 获取最近一次的 AI 回复内容
// ==========================================
String aiGetLastReply() {
    return g_lastReply;
}