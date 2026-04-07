#pragma once
#include <lvgl.h>
#include <stdint.h>

// 前向声明，避免与系统数据总线产生头文件循环依赖
struct SensorData;
struct SystemState;

// ==========================================
// 蓝牙信息页面接口 (Bluetooth Info Page)
// 职责：展示本机蓝牙状态、MAC 地址及连接设备信息
// ==========================================

// 创建蓝牙页面内容对象 (满足独立测试与创建需求)
// [返回值]：返回该页面的底层容器对象指针
lv_obj_t* uiBluetoothInfoPageCreate();

// 刷新蓝牙页面数据
// [作用]：由 ui_manager 在主循环中高频调用。负责将最新的蓝牙状态
// (如是否开启、连接状态、本机 MAC) 渲染刷新到界面的文本标签上。
void uiBluetoothInfoPageUpdate(const SensorData &data, const SystemState &state);

// ==========================================
// ⚠️ UI 管理器生命周期与路由契约
// 为了配合 ui_manager 的压栈路由机制，必须提供以下标准接口
// ==========================================

// 初始化蓝牙信息页面
// [参数]：parent - ui_manager 提供的内容区容器 (frame.content)
void uiBluetoothInfoPageInit(lv_obj_t *parent);

// 销毁蓝牙信息页面
// [作用]：在页面被切走时调用，用于安全清理内部对象指针，防止内存泄漏
void uiBluetoothInfoPageDeinit();

// 处理蓝牙页面的按键事件
// [参数]：key - 摇杆按键枚举
// [返回值]：返回期望跳转的目标页面 ID。纯展示页只需返回当前页 ID。
int uiBluetoothInfoPageHandleKey(uint32_t key);

// 语言切换后刷新静态标签文字
void uiBluetoothInfoPageRefreshLang();