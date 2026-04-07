#pragma once
#include <lvgl.h>
#include <stdint.h>

// 前向声明数据结构，避免与 app_data.h 循环依赖
struct SensorData;
struct SystemState;

// ==========================================
// 📋 主菜单页面接口 (Menu Page)
// ==========================================

// 创建菜单页面内容对象 (满足你的单独创建需求)
// [返回值]：返回菜单的底层容器对象指针
lv_obj_t* uiMenuPageCreate();

// 设置当前选中的菜单项索引
// [作用]：在用户通过物理摇杆上下按动时，或者从别的页面退回菜单时，
// 强制高亮指定序号的列表项，并赋予 Pocket PC 的经典深蓝底色。
// [参数]：index - 菜单项的数组下标 (0 开始)
void uiMenuPageSetSelected(uint8_t index);

// 获取当前选中的菜单项索引
// [作用]：当用户按下“确认(OK)”键时，调用此函数获知目前光标停留在哪个选项上，
// 从而决定跳转到哪一个具体的业务子页面。
// [返回值]：当前选中的菜单项下标
uint8_t uiMenuPageGetSelected();

// ==========================================
// ⚠️ UI 管理器生命周期与路由契约
// 必须包含以下四个接口，否则 ui_manager.cpp 编译将失败
// ==========================================

// 初始化主菜单页面
// [参数]：parent - 挂载此页面的标准内容区容器
void uiMenuPageInit(lv_obj_t *parent);

// 销毁主菜单页面
// [作用]：释放菜单列表指针，清理高亮状态，防止内存泄漏
void uiMenuPageDeinit();

// 刷新主菜单数据
// [作用]：主菜单主要用于静态导航，通常不需要高频刷新数据。保留此接口以满足统一契约。
void uiMenuPageUpdate(const SensorData &data, const SystemState &state);

// 处理主菜单的按键事件 (核心逻辑)
// [参数]：key - 摇杆按键枚举 (如 UI_KEY_UP, UI_KEY_DOWN, UI_KEY_OK)
// [返回值]：返回期望跳转的目标页面 ID (如 SENSOR_PAGE)。如果不跳转，则返回 MENU_PAGE。
int uiMenuPageHandleKey(uint32_t key);