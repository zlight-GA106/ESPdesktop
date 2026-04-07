#pragma once
#include <lvgl.h>
#include <stdint.h>

// 前向声明数据结构，避免与 app_data.h 产生互相包含的头文件灾难
struct SensorData;
struct SystemState;

// ==========================================
// ⚙️ 设置主菜单页面接口 (Settings Menu Page)
// ==========================================

// 创建设置菜单页面内容对象 (满足独立创建需求)
// [返回值]：返回菜单的底层容器对象指针
lv_obj_t* uiSettingsMenuPageCreate();

// 设置当前选中的设置项索引
// [作用]：在用户通过物理摇杆上下按动时，强制高亮指定序号的列表项，
// 并赋予 Pocket PC 的经典深蓝底色焦点样式。
// [参数]：index - 菜单项的数组下标 (0 开始)
void uiSettingsMenuPageSetSelected(uint8_t index);

// 获取当前选中的设置项索引
// [作用]：当用户按下“确认(OK)”键时，供内部逻辑或外部查询当前光标位置，
// 从而决定跳入哪个具体的设置子页面 (如亮度调节或 Wi-Fi 信息)。
// [返回值]：当前选中的菜单项下标
uint8_t uiSettingsMenuPageGetSelected();

// ==========================================
// ⚠️ UI 管理器生命周期与路由契约
// 必须包含以下四个标准接口，否则 ui_manager 将无法对它进行压栈调度
// ==========================================

// 初始化设置菜单页面
// [参数]：parent - 挂载此页面的标准内容区容器 (frame.content)
void uiSettingsMenuPageInit(lv_obj_t *parent);

// 销毁设置菜单页面
// [作用]：清理列表项指针，释放状态，防止野指针导致系统崩溃
void uiSettingsMenuPageDeinit();

// 刷新设置菜单数据
// [作用]：通常静态菜单不需要高频刷新，保留此接口以满足 UI 框架的统一多态契约。
void uiSettingsMenuPageUpdate(const SensorData &data, const SystemState &state);

// 处理设置菜单的按键事件
// [参数]：key - 摇杆按键枚举 (如 UI_KEY_UP, UI_KEY_DOWN, UI_KEY_OK)
// [返回值]：返回期望跳转的目标页面 ID (如 BRIGHTNESS_PAGE)。如果不跳转，则返回当前 ID。
int uiSettingsMenuPageHandleKey(uint32_t key);

// 语言切换后刷新内容区静态文字
void uiSettingsMenuPageRefreshLang();