#pragma once
#include <lvgl.h>
#include <stdint.h>

// 前向声明，避免与系统数据总线产生互相包含的灾难
struct SensorData;
struct SystemState;

// ==========================================
// 📶 Wi-Fi 信息页面接口 (Wi-Fi Info Page)
// ==========================================

// 创建 Wi-Fi 页面对象 (满足独立测试与创建需求)
// [返回值]：返回该页面的底层容器对象指针
lv_obj_t* uiWifiInfoPageCreate();

// 刷新 Wi-Fi 页面数据
// [作用]：由 ui_manager 在主循环中高频定时调用。负责从 state 中提取
// 最新的网络状态 (如 IP 地址、SSID、信号强度 RSSI) 并更新到界面的文本标签上。
void uiWifiInfoPageUpdate(const SensorData &data, const SystemState &state);

// ==========================================
// ⚠️ UI 管理器生命周期与路由契约
// 为了配合 ui_manager 的压栈路由机制，必须提供以下标准接口
// ==========================================

// 初始化 Wi-Fi 信息页面
// [参数]：parent - ui_manager 提供的内容区容器 (frame.content)
void uiWifiInfoPageInit(lv_obj_t *parent);

// 销毁 Wi-Fi 信息页面
// [作用]：页面退出时安全释放内部指针，防止内存泄漏
void uiWifiInfoPageDeinit();

// 处理 Wi-Fi 页面的按键事件
// [参数]：key - 摇杆按键枚举
// [返回值]：返回期望跳转的目标页面 ID。本页面是纯展示的末端页面，
// 除了被全局拦截的返回键，遇到任何按键都应返回当前页 ID 维持原状。
int uiWifiInfoPageHandleKey(uint32_t key);

// 语言切换后刷新静态标签文字
void uiWifiInfoPageRefreshLang();