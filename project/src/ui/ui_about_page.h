#pragma once
#include <lvgl.h>
#include <stdint.h>

// 前向声明数据结构，避免与系统核心数据白板互相包含导致编译死锁
struct SensorData;
struct SystemState;

// ==========================================
// ℹ️ 关于本机页面接口 (About Page)
// 职责：展示设备软硬件信息与系统状态
// ==========================================

// 创建关于页面内容对象 (满足你明确要求的声明)
// [返回值]：返回该页面的底层容器对象指针，常用于独立调试
lv_obj_t* uiAboutPageCreate();

// 刷新关于页面数据 (满足你明确要求的声明)
// [作用]：由 ui_manager 在主循环中定时调用。负责从 state 中提取
// 如剩余可用内存 (Free Heap) 等动态数据并刷新界面。
void uiAboutPageUpdate(const SensorData &data, const SystemState &state);

// ==========================================
// ⚠️ UI 管理器生命周期与路由契约
// 为了配合 ui_manager.cpp 中已经写死的压栈路由机制，必须补充以下标准接口。
// ==========================================

// 初始化关于本机页面
// [参数]：parent - ui_manager 提供的内容区容器 (frame.content)
void uiAboutPageInit(lv_obj_t *parent);

// 销毁关于本机页面
// [作用]：在页面退出时安全释放内部指针，防止内存泄漏
void uiAboutPageDeinit();

// 处理关于页面的按键事件
// [参数]：key - 摇杆按键枚举
// [返回值]：返回期望跳转的目标页面 ID。本页面是纯展示页，遇到除了全局返回外的按键均维持原状。
int uiAboutPageHandleKey(uint32_t key);