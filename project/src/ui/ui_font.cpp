#include "ui_font.h"

// ======================================================
// ui_font.cpp — 字体抽象层实现
//
// 统一字体：ui_cn_font16_full (16px, BPP=2)
//   来源：src/fonts/ui_cn_font16_full.c
//   由 LVGL Font Converter 生成，含 ASCII + 约 2500 常用汉字
//
// 所有接口均返回同一字体，通过函数名区分未来可能的专用字号。
// ======================================================

#include "../fonts/ui_fonts.h"

// ── 统一接口实现 ─────────────────────────────────────────
const lv_font_t* ui_font_default()    { return &ui_cn_font16_full; }
const lv_font_t* uiFontGetDefault()   { return &ui_cn_font16_full; }
const lv_font_t* uiFontGetTitle()     { return &ui_cn_font16_full; }
const lv_font_t* uiFontGetSmall()     { return &ui_cn_font16_full; }
