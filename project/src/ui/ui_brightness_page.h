#pragma once
#include <lvgl.h>

// 前向声明，避免与系统数据总线产生互相包含的灾难
struct SensorData;
struct SystemState;

// ==========================================
// 💡 屏幕亮度调节页面接口 (Brightness Page)
// 职责：提供一个可通过摇杆左右调节的滑块/进度条 UI
// ==========================================

// 创建亮度页面对象 (满足你明确要求的声明)
// [返回值]：返回该页面的底层容器对象指针
lv_obj_t* uiBrightnessPageCreate();

// 独立更新亮度界面的数值 (满足你明确要求的声明)
// [作用]：直接向界面注入一个具体的亮度值 (如 0-100)，更新滑块位置
// [参数]：brightnessValue - 新的亮度百分比
void uiBrightnessPageUpdate(int brightnessValue);

// 步进调节亮度界面的数值 (满足你明确要求的声明)
// [作用]：接收摇杆左/右指令后，按固定的步长 (如 +10 或 -10) 增减当前界面上的亮度值
// [参数]：delta - 变化量 (正数为增加，负数为减少)
void uiBrightnessPageAdjust(int delta);

// ==========================================
// ⚠️ UI 管理器生命周期与路由契约
// 为了配合 ui_manager.cpp 中已经写死的压栈路由机制，必须补充以下标准接口。
// 注意：此处的 Update 构成了 C++ 的函数重载 (Overload)。
// ==========================================

// 初始化亮度调节页面
// [参数]：parent - ui_manager 提供的内容区容器 (frame.content)
void uiBrightnessPageInit(lv_obj_t *parent);

// 销毁亮度调节页面
// [作用]：在页面退出时安全释放内部指针，防止内存泄漏
void uiBrightnessPageDeinit();

// 刷新亮度页面数据 (满足 ui_manager 统一调用的契约)
// [作用]：全局主循环调用此函数，内部可提取 state.brightness 并转调 uiBrightnessPageUpdate(int)
void uiBrightnessPageUpdate(const SensorData &data, const SystemState &state);

// 处理亮度页面的按键事件
// [参数]：key - 摇杆按键枚举
// [返回值]：返回期望跳转的目标页面 ID。遇到左右按键时拦截并调用 Adjust，遇到 OK/返回 键则退出。
int uiBrightnessPageHandleKey(uint32_t key);

// 语言切换后刷新标签文字
void uiBrightnessPageRefreshLang();