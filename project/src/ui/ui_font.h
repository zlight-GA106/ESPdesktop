#pragma once
#include <lvgl.h>

// ======================================================
// ui_font.h — 统一字体接口
//
// 当前唯一字体：ui_cn_font16_full (16px, BPP=2)
//   来源：src/fonts/ui_cn_font16_full.c
//   含 ASCII (0x20–0x7F) + 约 2500 常用汉字
//
// ❗规范：所有控件必须通过以下接口获取字体，
//         禁止在控件代码中直接引用 &ui_cn_font16_full
// ======================================================

/**
 * @brief 统一默认字体（新接口，推荐使用）
 *        等同于 uiFontGetDefault()，后续扩展字号从此处分发
 */
const lv_font_t* ui_font_default();

/**
 * @brief 默认正文字体 (列表项、字段名、内容文字) — 16px
 */
const lv_font_t* uiFontGetDefault();

/**
 * @brief 标题栏字体 — 16px（当前无独立大字体，与正文同）
 */
const lv_font_t* uiFontGetTitle();

/**
 * @brief 状态栏字体 — 16px（当前无独立小字体，与正文同）
 */
const lv_font_t* uiFontGetSmall();
