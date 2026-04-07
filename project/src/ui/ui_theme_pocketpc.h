#pragma once
#include <lvgl.h>
#include "ui_font.h"  // 统一字体接口，不直接引用 lv_font_montserrat_14

// ==========================================
// 🎨 视觉主题与样式定义 (Pocket PC 2000 风格)
// 核心：直角无圆角、16色低色深质感、海军蓝高亮、哑光冷灰主调
// ==========================================

// 初始化所有 Pocket PC 风格的全局样式
// [作用]：在屏幕驱动加载完毕后，由 ui_frame 或主程序调用一次。
// 它会在内存中分配并配置好所有的 lv_style_t 对象。
void uiThemePocketPcInit();

// ==========================================
// 【全局复用样式对象声明】
// 加上 extern，让所有 include 此文件的页面都能直接给控件“穿上这套皮肤”
// ==========================================

/**
 * style_page_bg
 * [用途]：页面的最底层容器背景，铺满整个屏幕
 * [视觉]：浅灰色 #E0E0E0，绝对直角，无边框，无内边距
 * [适用]：Frame.root、内容区容器的底色
 */
extern lv_style_t style_page_bg;

/**
 * style_title_bar
 * [用途]：顶部标题栏容器的背景
 * [视觉]：海军蓝 #000080，撑满屏幕宽度，高度 22px
 * [适用]：Frame 中的标题栏容器底色
 */
extern lv_style_t style_title_bar;

/**
 * style_title_text
 * [用途]：顶栏中心的标题文字样式
 * [视觉]：纯白色 #FFFFFF，无背景，清晰可读
 * [适用]：Frame.titleLabel
 */
extern lv_style_t style_title_text;

/**
 * style_status_text
 * [用途]：标题栏下方的独立状态指示文字（如 "WiFi:-- MQTT:--"）
 * [视觉]：黑色文字，中灰底色，紧凑排版
 * [适用]：Frame.statusLabel
 * [说明]：独立一行，避免和标题栏按钮重叠
 */
extern lv_style_t style_status_text;

/**
 * style_button_normal
 * [用途]：按钮的普通状态样式
 * [视觉]：中灰色 #C0C0C0 底色，黑色 1px 硬边框，模拟 Win9x 凸起浮雕感
 * [适用]：Frame.backBtn、Frame.menuBtn 的默认状态
 * [交互]：搭配 style_button_pressed 形成按压反馈
 */
extern lv_style_t style_button_normal;

/**
 * style_button_pressed
 * [用途]：按钮被按下时的状态样式
 * [视觉]：深灰色 #808080 底色，黑色硬边框，模拟凹陷按压感
 * [原理]：降低背景色亮度，人眼会产生按键"陷下去"的错觉
 * [适用]：Frame.backBtn、Frame.menuBtn 的 LV_STATE_PRESSED
 */
extern lv_style_t style_button_pressed;

/**
 * style_content_bg
 * [用途]：内容区容器的背景
 * [视觉]：浅灰色 #E0E0E0，与页面背景同色以保持统一
 * [适用]：Frame.content 的底色
 * [说明]：单独定义方便未来对内容区做特殊处理
 */
extern lv_style_t style_content_bg;

/**
 * style_list_item
 * [用途]：菜单列表项的普通未选中状态
 * [视觉]：浅灰底 #E0E0E0 + 黑字，满宽平铺，无圆角
 * [适用]：uiMenuPage 中未选中的行
 */
extern lv_style_t style_list_item;

/**
 * style_list_selected
 * [用途]：菜单列表项的选中高亮状态
 * [视觉]：深蓝 #000080 底 + 白字，与标题栏同色系形成焦点
 * [适用]：uiMenuPage 中当前选中的行
 */
extern lv_style_t style_list_selected;