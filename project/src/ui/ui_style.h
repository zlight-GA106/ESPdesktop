#pragma once
#include <lvgl.h>
#include "ui_font.h"   // ui_font_default() 的权威声明来源

// ==========================================================
// ui_style.h — 统一样式管理模块
//
// 职责：
//   1. 初始化全局文本样式 g_style_text（供所有控件复用）
//   2. 在 displayDriverBegin() 之后由 ui_style_init() 调用一次
//
// 字体获取接口：ui_font_default() 定义于 ui_font.h / ui_font.cpp
// ==========================================================

// ── 全局复用样式对象（需要使用的文件加 extern 引用） ──────
extern lv_style_t g_style_text;

/**
 * @brief 初始化全局 UI 样式（必须在 lv_init() 之后调用）
 *        设置默认字体、颜色、LVGL 默认主题。
 *        打印字体加载日志便于调试。
 */
void ui_style_init(void);
