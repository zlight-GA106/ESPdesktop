#pragma once
#include <lvgl.h>
#include <stdint.h>

// 前向声明，避免与系统数据总线产生互相包含的灾难
struct SensorData;
struct SystemState;

// ==========================================
// 📻 网络电台页面接口 (Web Radio Page)
// 职责：提供电台列表选择，并展示流媒体播放状态
// ==========================================

// 创建电台页面对象 (满足你明确要求的声明)
// [返回值]：返回该页面的底层容器对象指针
lv_obj_t* uiRadioPageCreate();

// 独立刷新电台页面的内部状态 (满足你明确要求的声明)
// [作用]：用于在不依赖全局系统状态的情况下，单独刷新电台列表的 UI 或播放动画
void uiRadioPageUpdate();

// 设置当前选中的电台频道索引 (满足你明确要求的声明)
// [作用]：在用户通过物理摇杆上下按动时，强制高亮指定序号的电台项
// [参数]：index - 电台列表的数组下标 (0 开始)
void uiRadioPageSetSelected(uint8_t index);

// 获取当前选中的电台频道索引 (满足你明确要求的声明)
// [作用]：当用户按下“确认(OK)”键时，获取当前光标停留在哪个频道，从而向底层下发播放对应 URL 的指令
// [返回值]：当前选中的频道下标
uint8_t uiRadioPageGetSelected();

// ==========================================
// ⚠️ UI 管理器生命周期与路由契约
// 为了配合 ui_manager.cpp 中已经写死的压栈路由机制，必须补充以下标准接口。
// 注意：此处的 Update 构成了 C++ 的函数重载 (Overload)。
// ==========================================

// 初始化网络电台页面
// [参数]：parent - ui_manager 提供的内容区容器 (frame.content)
void uiRadioPageInit(lv_obj_t *parent);

// 销毁网络电台页面
// [作用]：在页面退出时安全释放内部指针，防止内存泄漏
void uiRadioPageDeinit();

// 刷新电台页面全局数据 (满足 ui_manager 统一调用的契约)
// [作用]：全局主循环高频调用，可用于读取 state 中的音频播放状态并转调无参的 Update
void uiRadioPageUpdate(const SensorData &data, const SystemState &state);

// 处理电台页面的按键事件
// [参数]：key - 摇杆按键枚举
// [返回值]：返回期望跳转的目标页面 ID。遇到上下键切换频道，OK键触发播放，返回键退出并由 manager 停播。
int uiRadioPageHandleKey(uint32_t key);