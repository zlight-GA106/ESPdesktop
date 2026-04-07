#pragma once
#include <lvgl.h>
#include <stdint.h>

// 前向声明，避免与 app_data.h 产生头文件循环依赖
struct SensorData;
struct SystemState;

// ==========================================
// 📊 传感器详情页面接口 (Sensor Page)
// 职责：集中展示温湿度、光照、雷达等硬件采集的实时数据
// ==========================================

// 创建传感器页面对象 (满足独立创建需求)
// [返回值]：返回该页面的底层容器对象指针
lv_obj_t* uiSensorPageCreate();

// 刷新传感器页面数据
// [作用]：由 ui_manager 在主循环中高频调用。负责将最新的 SensorData
// 映射到具体的文本或进度条控件上，让界面数据实时跳动。
// [参数]：data - 包含温湿度、光照、雷达等最新值的结构体
void uiSensorPageUpdate(const SensorData &data, const SystemState &state);

// ==========================================
// ⚠️ UI 管理器生命周期与路由契约
// 为了配合 ui_manager 的压栈路由机制，必须提供以下标准接口
// ==========================================

// 初始化传感器页面
// [参数]：parent - ui_manager 提供的内容区容器 (frame.content)
void uiSensorPageInit(lv_obj_t *parent);

// 销毁传感器页面
// [作用]：在页面退出时安全释放内部指针，防止内存泄漏
void uiSensorPageDeinit();

// 处理传感器页面的按键事件
// [参数]：key - 摇杆按键枚举
// [返回值]：返回期望跳转的目标页面 ID。对于纯展示页，除了返回键会被
// 全局拦截外，通常直接返回当前页 ID 即可。
int uiSensorPageHandleKey(uint32_t key);