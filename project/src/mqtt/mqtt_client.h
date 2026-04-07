#pragma once
#include <Arduino.h>

// 前向声明数据结构，避免与系统核心数据白板互相包含导致编译死锁
struct SensorData;
struct SystemState;

// ==========================================
// ☁️ MQTT 客户端接口 (MQTT Client)
// 职责：管理设备与 MQTT Broker 的通信，实现数据上报与指令下发
// ==========================================

// 初始化 MQTT 客户端
// [作用]：配置 MQTT Broker 的 IP 地址、端口号，并绑定底层的 Wi-Fi 客户端实例。
// 此函数仅做配置准备，不发起真正的网络握手。
// [返回值]：初始化成功返回 true，失败返回 false。
bool mqttClientBegin();

// 执行 MQTT 连接
// [作用]：向配置好的 MQTT Broker 发起建立连接请求，可包含设备鉴权 (Client ID, User, Pass) 
// 和遗嘱消息 (LWT) 设置。若已掉线，调用此函数可触发重连。
// [返回值]：连接并认证成功返回 true，超时或拒绝连接返回 false。
bool mqttClientConnect();

// 查询当前 MQTT 实时连接状态
// [作用]：供主循环 (Main Loop) 或 MQTT UI 状态页高频调用，
// 实时探测设备是否与云端保持着活跃的 TCP 连接。
// [返回值]：当前处于已连接状态返回 true，掉线或未连接返回 false。
bool mqttClientIsConnected();

// MQTT 客户端请求处理泵 (Tick)
// [作用]：必须在全局主循环 (Main Loop) 中持续高频调用。
// 它负责发送 Keep-Alive 心跳包以维持连接，并轮询接收云端下发的订阅消息。
// 如果不调用此函数，MQTT 连接将在几秒内被 Broker 强制断开！
void mqttClientLoop();

// 发布遥测数据 (Publish Telemetry)
// [作用]：将传感器读数和系统当前状态打包（通常转为 JSON 格式），
// 并发布 (Publish) 到特定的数据上报 Topic 中，供 Home Assistant 等中枢抓取。
// [参数]：data - 包含温湿度、光照等数据的结构体引用
// [参数]：state - 包含网络、AI 状态等系统运行数据的结构体引用
// [返回值]：成功推送到发送缓冲区返回 true，未连接或发送失败返回 false。
bool mqttClientPublishTelemetry(const SensorData &data, const SystemState &state);