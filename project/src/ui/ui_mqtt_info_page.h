#pragma once
#include <lvgl.h>
#include <stdint.h>

// 前向声明数据结构，避免与 app_data.h 产生头文件循环依赖
struct SensorData;
struct SystemState;

// ==========================================
// ☁️ MQTT 云端信息页面接口 (MQTT Info Page)
// 职责：展示设备与 MQTT 服务器的连接状态及核心参数
// ==========================================

// 创建 MQTT 页面内容对象 (满足独立测试与创建需求)
// [返回值]：返回该页面的底层容器对象指针
lv_obj_t* uiMqttInfoPageCreate();

// 刷新 MQTT 页面数据
// [作用]：由 ui_manager 在主循环中高频调用。负责将最新的 MQTT 连接状态
// (如是否已连接、服务器地址等) 渲染刷新到界面的文本标签上。
void uiMqttInfoPageUpdate(const SensorData &data, const SystemState &state);

// ==========================================
// ⚠️ UI 管理器生命周期与路由契约
// 为了配合 ui_manager 的压栈路由机制，必须提供以下标准接口
// ==========================================

// 初始化 MQTT 信息页面
// [参数]：parent - ui_manager 提供的内容区容器 (frame.content)
void uiMqttInfoPageInit(lv_obj_t *parent);

// 销毁 MQTT 信息页面
// [作用]：在页面被切走时调用，用于安全清理内部对象指针，防止内存泄漏
void uiMqttInfoPageDeinit();

// 处理 MQTT 页面的按键事件
// [参数]：key - 摇杆按键枚举
// [返回值]：返回期望跳转的目标页面 ID。对于纯展示页，除了返回键会被
// 全局拦截外，通常直接返回当前页 ID 即可维持原状。
int uiMqttInfoPageHandleKey(uint32_t key);

// 语言切换后刷新静态标签文字
void uiMqttInfoPageRefreshLang();