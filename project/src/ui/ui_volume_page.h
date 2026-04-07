#pragma once
#include <lvgl.h>

// 前向声明，避免与系统数据总线产生互相包含的灾难
struct SensorData;
struct SystemState;

// ==========================================
// 🔊 系统音量调节页面接口 (Volume Page)
// 职责：提供一个可通过摇杆调节的音量滑块/进度条 UI
// ==========================================

// 创建音量页面对象 (满足你明确要求的声明)
// [返回值]：返回该页面的底层容器对象指针
lv_obj_t* uiVolumePageCreate();

// 独立更新音量界面的数值 (满足你明确要求的声明)
// [作用]：直接向界面注入一个具体的音量值 (如 0-100)，更新滑块位置
// [参数]：volumeValue - 新的音量百分比
void uiVolumePageUpdate(int volumeValue);

// 步进调节音量界面的数值 (满足你明确要求的声明)
// [作用]：接收摇杆指令后，按固定的步长 (如 +5 或 -5) 增减当前界面上的音量值
// [参数]：delta - 变化量 (正数为增加，负数为减少)
void uiVolumePageAdjust(int delta);

// ==========================================
// ⚠️ UI 管理器生命周期与路由契约
// 为了配合 ui_manager.cpp 中已经写死的压栈路由机制，必须补充以下标准接口。
// 注意：此处的 Update 构成了 C++ 的函数重载 (Overload)。
// ==========================================

// 初始化音量调节页面
// [参数]：parent - ui_manager 提供的内容区容器 (frame.content)
void uiVolumePageInit(lv_obj_t *parent);

// 销毁音量调节页面
// [作用]：在页面退出时安全释放内部指针，防止内存泄漏
void uiVolumePageDeinit();

// 刷新音量页面数据 (满足 ui_manager 统一调用的契约)
// [作用]：全局主循环调用此函数，内部可提取 state.volume 并转调 uiVolumePageUpdate(int)
void uiVolumePageUpdate(const SensorData &data, const SystemState &state);

// 处理音量页面的按键事件
// [参数]：key - 摇杆按键枚举
// [返回值]：返回期望跳转的目标页面 ID。遇到调节按键时拦截并调用 Adjust，其余保持原状。
int uiVolumePageHandleKey(uint32_t key);

// 语言切换后刷新标签文字
void uiVolumePageRefreshLang();