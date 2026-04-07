#pragma once
#include <lvgl.h>

// 前向声明，避免与 app_data.h 产生互相包含的循环编译死锁
struct SensorData;
struct SystemState;

// ==========================================
// 🏠 待机主页接口 (Home Page)
// ==========================================

// 创建主页内容 (满足你的声明要求)
// [返回值]：返回主页的底层容器对象指针
lv_obj_t* uiHomePageCreate();

// 更新主页数据
// [作用]：由 ui_manager 在主循环中高频调用，负责将最新的温湿度、
// 时间戳等全局数据渲染刷新到界面的 Label 控件上。
void uiHomePageUpdate(const SensorData &data, const SystemState &state);

// ==========================================
// ⚠️ 补充声明：UI 管理器标准契约
// 为了适配我们上一节在 ui_manager.cpp 中写死的生命周期路由调度，
// 这里必须补充以下三个函数，否则编译时会直接报错。
// ==========================================

// 初始化主页
// [参数]：parent - ui_manager 分配给这个页面的标准内容区容器 (frame.content)
void uiHomePageInit(lv_obj_t *parent);

// 销毁主页
// [作用]：在页面被切走时调用，用于释放特定的内存或清除动画定时器。
void uiHomePageDeinit();

// 处理主页的按键事件
// [参数]：key - 摇杆按键枚举 (如 UI_KEY_RIGHT)
// [返回值]：返回期望跳转的目标页面 ID。如果不跳转，则返回当前页面 ID。
int uiHomePageHandleKey(uint32_t key);