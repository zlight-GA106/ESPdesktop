#include "mqtt_client.h"
#include "../../app_data.h"
#include "../storage/settings.h"

// 明确引用的第三方库：PubSubClient (作者: Nick O'Leary)
// 请在 PlatformIO 或 Arduino IDE 的库管理器中安装此库
#include <WiFi.h>
#include <PubSubClient.h>

// ==========================================
// ☁️ MQTT 客户端实现 (MQTT Client)
// ==========================================

// 实例化底层的 WiFi 客户端与 MQTT 客户端
static WiFiClient espClient;
static PubSubClient mqtt(espClient);

// --- Topic 设计 ---
// [Home Assistant 接入说明]
// 1. 状态 Topic (status): 用于 Home Assistant 判断设备是否在线 (可用性 availability_topic)。
//    发送 "online" 表示在线，通过 LWT (遗嘱机制) 在意外断网时由 Broker 自动代发 "offline"。
// 2. 遥测 Topic (telemetry): 用于上报温湿度等传感器数据。Home Assistant 可以在 configuration.yaml 
//    中使用 value_template: "{{ value_json.temp }}" 来提取数值。
static const char* TOPIC_STATUS    = "aiot/device01/status";
static const char* TOPIC_TELEMETRY = "aiot/device01/telemetry";

// 静态存储 Broker 地址，防止 String 销毁导致底层的 const char* 指针悬空
static String currentBroker = "";

/*
 * [最小 Discovery 思路 (无需手写 yaml 自动发现)]
 * Home Assistant 支持 MQTT Auto Discovery。
 * 如果你想实现自动发现，只需在 mqttClientConnect() 成功后，向特定的配置 Topic 发布一段固定的 JSON。
 * 例如：向 topic `homeassistant/sensor/device01_temp/config`
 * 发布 JSON：
 * {
 * "name": "Pocket PC Temperature",
 * "state_topic": "aiot/device01/telemetry",
 * "value_template": "{{ value_json.temp }}",
 * "unit_of_measurement": "°C",
 * "device_class": "temperature",
 * "unique_id": "device01_temp"
 * }
 * HA 收到该消息后，会自动在设备列表中创建一个带有温度图标的实体，完全免干预！
 * (为保持最小可运行版本，本文件的主逻辑暂不执行 Discovery 广播)
 */

// ==========================================
// [接口] 初始化 MQTT 客户端
// ==========================================
bool mqttClientBegin() {
    // 设置内部缓冲区大小以支持较长的 JSON 遥测报文 (默认通常只有 256)
    mqtt.setBufferSize(512);
    Serial.println("[MQTT] 客户端初始化完成，等待连接触发");
    return true;
}

// ==========================================
// [接口] 执行 MQTT 连接
// ==========================================
bool mqttClientConnect() {
    // 1. 从 Settings 模块动态读取最新的 Broker 地址
    currentBroker = settingsGetMqttBroker();
    if (currentBroker.length() == 0 || currentBroker == "0.0.0.0") {
        Serial.println("[MQTT] 错误：未配置 Broker 地址，跳过连接。");
        return false;
    }

    // 设置服务器地址和默认端口 1883
    mqtt.setServer(currentBroker.c_str(), 1883);

    Serial.printf("[MQTT] 正在连接至 Broker: %s\n", currentBroker.c_str());

    // 2. 发起连接，并配置 LWT (遗嘱消息)
    // 参数：ClientID, 遗嘱Topic, QoS, Retain, 遗嘱内容
    // 这样设备意外断电时，Broker 会向所有订阅者广播 "offline"
    String clientId = "ESPDesktop-" + String(WiFi.macAddress());
    bool success = mqtt.connect(clientId.c_str(), TOPIC_STATUS, 0, true, "offline");

    if (success) {
        Serial.println("[MQTT] 连接成功！");
        // 连上后立刻发布一条保留消息 (Retain=true)，声明自己在线
        mqtt.publish(TOPIC_STATUS, "online", true);
        
        // 预留：此处可以加入 mqtt.subscribe("aiot/device01/command"); 用于接收云端控制指令
        return true;
    } else {
        Serial.printf("[MQTT] 连接失败，错误状态码 (rc): %d\n", mqtt.state());
        return false;
    }
}

// ==========================================
// [接口] 查询当前 MQTT 实时连接状态
// ==========================================
bool mqttClientIsConnected() {
    return mqtt.connected();
}

// ==========================================
// [接口] MQTT 客户端请求处理泵 (Tick)
// ==========================================
void mqttClientLoop() {
    // 处理底层的心跳保活 (Ping/Pong) 和接收订阅消息。必须在 main loop 中高频调用！
    if (mqtt.connected()) {
        mqtt.loop();
    }
}

// ==========================================
// [接口] 发布遥测数据 (Publish Telemetry)
// ==========================================
bool mqttClientPublishTelemetry(const SensorData &data, const SystemState &state) {
    if (!mqtt.connected()) {
        return false;
    }

    // 构造极简的 JSON 字符串进行上报
    // 这里使用原生的 snprintf 以摆脱对 ArduinoJson 的强制依赖，保持项目轻量化
    char payload[256];
    snprintf(payload, sizeof(payload), 
        "{\"temp\":%.1f,\"hum\":%.1f,\"light\":%d,\"pir\":%d,\"sound\":%d,\"wifi_ip\":\"%s\"}",
        data.temperature,
        data.humidity,
        data.lightLevel,
        data.humanDetected ? 1 : 0,
        data.soundLevel,
        state.wifiIp
    );

    // 发布到遥测 Topic (非保留消息)
    bool result = mqtt.publish(TOPIC_TELEMETRY, payload, false);
    
    if (result) {
        Serial.println("[MQTT] 遥测数据上报成功");
    } else {
        Serial.println("[MQTT] 遥测数据上报失败 (缓冲区满或网络断开)");
    }
    
    return result;
}