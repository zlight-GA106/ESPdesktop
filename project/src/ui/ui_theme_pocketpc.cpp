#include <Arduino.h>
#include "ui_theme_pocketpc.h"

// ==========================================
// 🎨 Pocket PC 2000 主题实现
// ==========================================

// 定义是 7 + 2 = 9 个全局样式实例
lv_style_t style_page_bg;
lv_style_t style_title_bar;
lv_style_t style_title_text;
lv_style_t style_status_text;
lv_style_t style_button_normal;
lv_style_t style_button_pressed;
lv_style_t style_content_bg;
lv_style_t style_list_item;
lv_style_t style_list_selected;

// --- Pocket PC 2000 / Win CE 3.0 核心色卡 ---
#define COLOR_NAVY_BLUE   lv_color_hex(0x000080)  // 高亮主色：海军蓝
#define COLOR_LIGHT_GRAY  lv_color_hex(0xE0E0E0)  // 屏幕底色：浅冷灰
#define COLOR_MID_GRAY    lv_color_hex(0xC0C0C0)  // 控件表面：中度灰
#define COLOR_DARK_GRAY   lv_color_hex(0x808080)  // 按压凹陷：深灰色
#define COLOR_BLACK       lv_color_hex(0x000000)  // 边框文字：纯黑
#define COLOR_WHITE       lv_color_hex(0xFFFFFF)  // 白色文字：纯白

// ==========================================
// [接口] 初始化所有 Pocket PC 样式
// ==========================================
void uiThemePocketPcInit() {
    // ========== 1. 页面背景 (style_page_bg) ==========
    // [用途]：屏幕根容器和内容区的底色
    // [效果]：浅灰色背景，为整个UI提供统一的视觉基调
    lv_style_init(&style_page_bg);
    lv_style_set_bg_color(&style_page_bg, COLOR_LIGHT_GRAY);
    lv_style_set_bg_opa(&style_page_bg, LV_OPA_COVER);
    lv_style_set_radius(&style_page_bg, 0);           // 绝对直角
    lv_style_set_border_width(&style_page_bg, 0);    // 无边框
    lv_style_set_pad_all(&style_page_bg, 0);         // 无内边距
    lv_style_set_text_color(&style_page_bg, COLOR_BLACK);

    // ========== 2. 标题栏 (style_title_bar) ==========
    // [用途]：顶部状态栏容器
    // [效果]：海军蓝背景，占据屏幕宽度和固定高度，为系统级信息提供视觉锚点
    lv_style_init(&style_title_bar);
    lv_style_set_bg_color(&style_title_bar, COLOR_NAVY_BLUE);
    lv_style_set_bg_opa(&style_title_bar, LV_OPA_COVER);
    lv_style_set_radius(&style_title_bar, 0);
    lv_style_set_border_width(&style_title_bar, 0);
    // 为内部按钮和文字留出极小的呼吸空间
    lv_style_set_pad_left(&style_title_bar, 2);
    lv_style_set_pad_right(&style_title_bar, 2);
    lv_style_set_pad_top(&style_title_bar, 1);
    lv_style_set_pad_bottom(&style_title_bar, 1);

    // ========== 3. 标题文本 (style_title_text) ==========
    // [用途]：标题栏中间的页面名称文字
    // [效果]：纯白色，与海军蓝标题栏形成极高对比度
    lv_style_init(&style_title_text);
    lv_style_set_text_color(&style_title_text, COLOR_WHITE);
    lv_style_set_text_font(&style_title_text, uiFontGetTitle());    // 通过 ui_font 统一接口取字体
    lv_style_set_bg_opa(&style_title_text, LV_OPA_TRANSP);  // 透明背景
    lv_style_set_border_width(&style_title_text, 0);
    lv_style_set_pad_all(&style_title_text, 0);

    // ========== 4. 状态文本 (style_status_text) ==========
    // [用途]：标题栏下方的独立状态栏文字 (如"WiFi:-- MQTT:--")
    // [视觉]：黑色文字，紧凑排版，标题栏与内容区之间的系统状态指示
    // [原因]：小屏幕下每个像素都金贵，不能浪费
    lv_style_init(&style_status_text);
    lv_style_set_text_color(&style_status_text, COLOR_BLACK);
    lv_style_set_text_font(&style_status_text, uiFontGetSmall());   // 状态栏紧凑字体
    lv_style_set_bg_color(&style_status_text, COLOR_MID_GRAY);          // 中灰底，与内容区浅灰区分
    lv_style_set_bg_opa(&style_status_text, LV_OPA_COVER);
    lv_style_set_border_width(&style_status_text, 0);
    // [FIX] 垂直 pad=0：font_14 行高恰好 = statusBar 高度14px
    // 任何垂直 padding 都会裁掉字符底部像素；水平保留 2px 防贴边
    lv_style_set_pad_top(&style_status_text, 0);
    lv_style_set_pad_bottom(&style_status_text, 0);
    lv_style_set_pad_left(&style_status_text, 2);
    lv_style_set_pad_right(&style_status_text, 2);

    // ========== 5. 按钮普通态 (style_button_normal) ==========
    // [用途]：返回/菜单按钮的默认状态
    // [视觉]：中灰色底 + 黑色硬边框，营造 Win9x 凸起感
    // [原理]：中灰底色相对于浅灰背景有对比，黑边框强化定义，形成浮雕效果
    lv_style_init(&style_button_normal);
    lv_style_set_bg_color(&style_button_normal, COLOR_MID_GRAY);
    lv_style_set_bg_opa(&style_button_normal, LV_OPA_COVER);
    lv_style_set_text_color(&style_button_normal, COLOR_BLACK);
    lv_style_set_radius(&style_button_normal, 0);  // 直角
    lv_style_set_border_width(&style_button_normal, 1);
    lv_style_set_border_color(&style_button_normal, COLOR_BLACK);
    lv_style_set_pad_all(&style_button_normal, 2);

    // ========== 6. 按钮按下态 (style_button_pressed) ==========
    // [用途]：返回/菜单按钮被按下时的视觉反馈
    // [视觉]：深灰色底 + 黑色硬边框，营造凹陷感
    // [原理]：色亮度骤降，人眼瞬间产生"按键陷下去"的错觉
    // [设计]：这是经典2D UI中唯一实现物理反馈的方法
    lv_style_init(&style_button_pressed);
    lv_style_set_bg_color(&style_button_pressed, COLOR_DARK_GRAY);
    lv_style_set_bg_opa(&style_button_pressed, LV_OPA_COVER);
    lv_style_set_text_color(&style_button_pressed, COLOR_BLACK);
    lv_style_set_radius(&style_button_pressed, 0);
    lv_style_set_border_width(&style_button_pressed, 1);
    lv_style_set_border_color(&style_button_pressed, COLOR_BLACK);
    lv_style_set_pad_all(&style_button_pressed, 2);

    // ========== 7. 内容区背景 (style_content_bg) ==========
    // [用途]：主内容区容器的背景
    // [效果]：与页面背景同色，保持视觉统一
    // [好处]：单独定义便于未来对内容区的特殊处理（如滚动条样式）
    lv_style_init(&style_content_bg);
    lv_style_set_bg_color(&style_content_bg, COLOR_LIGHT_GRAY);
    lv_style_set_bg_opa(&style_content_bg, LV_OPA_COVER);
    lv_style_set_radius(&style_content_bg, 0);
    lv_style_set_border_width(&style_content_bg, 0);
    lv_style_set_pad_all(&style_content_bg, 0);
    lv_style_set_text_color(&style_content_bg, COLOR_BLACK);

    // ========== 8. 列表项普通态 (style_list_item) ==========
    // [用途]：菜单页列表项的默认未选中状态
    // [视觉]：浅灰底色、黑色文字，满宽平铺，无圆角无边框
    // [设计]：和内容区底色相同，不选中时几乎“消失”进内容区
    lv_style_init(&style_list_item);
    lv_style_set_bg_color(&style_list_item, COLOR_LIGHT_GRAY);
    lv_style_set_bg_opa(&style_list_item, LV_OPA_COVER);
    lv_style_set_text_color(&style_list_item, COLOR_BLACK);
    lv_style_set_radius(&style_list_item, 0);
    lv_style_set_border_width(&style_list_item, 0);
    lv_style_set_pad_top(&style_list_item, 4);
    lv_style_set_pad_bottom(&style_list_item, 4);
    lv_style_set_pad_left(&style_list_item, 6);
    lv_style_set_pad_right(&style_list_item, 0);

    // ========== 9. 列表项选中态 (style_list_selected) ==========
    // [用途]：菜单页列表项的选中高亮
    // [视觉]：深蓝底 #000080 + 白色文字，与页面标题栏同色系形成视觉层次的焦点
    // [原理]：读者的眼睛天生对高对比深蓝/白色组合最敏感，它是经典 Windows CE 选中配色
    lv_style_init(&style_list_selected);
    lv_style_set_bg_color(&style_list_selected, COLOR_NAVY_BLUE);
    lv_style_set_bg_opa(&style_list_selected, LV_OPA_COVER);
    lv_style_set_text_color(&style_list_selected, COLOR_WHITE);
    lv_style_set_radius(&style_list_selected, 0);
    lv_style_set_border_width(&style_list_selected, 0);
    lv_style_set_pad_top(&style_list_selected, 4);
    lv_style_set_pad_bottom(&style_list_selected, 4);
    lv_style_set_pad_left(&style_list_selected, 6);
    lv_style_set_pad_right(&style_list_selected, 0);

    Serial.println("[THEME] all styles OK (incl. list)");
}
