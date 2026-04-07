// ==========================================================
// ui_style.cpp — 统一样式管理模块实现
//
// 字体来源: src/fonts/ui_cn_font16_full.c
//   BPP=2, 16px, ASCII (0x20–0x7F) + 约 2500 常用汉字
//   由 LVGL Font Converter 生成
// ==========================================================

#include "ui_style.h"
#include "ui_font.h"      // ui_font_default() 在此定义，避免重复符号
#include <Arduino.h>

// ── 全局文本样式实例（供控件直接 lv_obj_add_style 复用） ──
lv_style_t g_style_text;

// ──────────────────────────────────────────────────────────
// ui_style_init() — 初始化全局样式（必须在 lv_init() 后调用）
// ──────────────────────────────────────────────────────────
void ui_style_init(void)
{
    // ── 打印字体加载日志（答辩 / 调试用） ──────────────────
    printf("[UI] Font system initialized\n");
    LV_LOG_USER("Default font: %p", (void*)ui_font_default());

    // ── 初始化全局文本样式 ──────────────────────────────────
    lv_style_init(&g_style_text);
    lv_style_set_text_font(&g_style_text, ui_font_default());
    lv_style_set_text_color(&g_style_text, lv_color_white());

    // ── 设置 LVGL 默认主题（蓝青双色，亮色模式，中文字体） ──
    lv_theme_t *th = lv_theme_default_init(
        lv_disp_get_default(),
        lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_CYAN),
        false,
        ui_font_default());
    lv_disp_set_theme(lv_disp_get_default(), th);

    // ── 强制根屏幕继承字体（防 lv_scr_act 使用系统默认字体） ─
    lv_obj_set_style_text_font(lv_scr_act(), ui_font_default(), 0);

    printf("[UI] ui_style_init done, theme applied\n");
}
