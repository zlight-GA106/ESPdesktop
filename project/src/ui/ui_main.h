#pragma once
#include <lvgl.h>

// ======================================================
// ui_main.h — 主 UI 系统入口
//
// 管理 PAGE_HOME / PAGE_SETTINGS / PAGE_INFO 三页系统。
// 页面按需创建、切换后通过 lv_async_call 异步销毁旧页，
// 避免在 LVGL 回调栈中 lv_obj_del 自身导致崩溃。
//
// 用法:
//   displayDriverBegin();  // 初始化 LVGL + 驱动
//   ui_main_init();        // 加载首页（主界面）
// ======================================================

/** @brief 页面枚举 */
typedef enum {
    PAGE_HOME     = 0,   ///< 主界面 — 时间/状态 + [设置][信息]
    PAGE_SETTINGS = 1,   ///< 设置    — WiFi/时间/系统 列表 + [返回]
    PAGE_INFO     = 2,   ///< 设备信息 — 名称/版本/芯片  + [返回]
} ui_page_t;

/**
 * @brief 初始化 UI 主系统，加载首页 PAGE_HOME
 *        必须在 displayDriverBegin() / lv_init() 之后调用
 */
void ui_main_init(void);

/**
 * @brief 切换到指定页面
 *        旧页面通过 lv_async_call 延迟删除，
 *        保证在按钮回调返回后再释放对象，不会重入崩溃
 */
void ui_switch_page(ui_page_t page);
