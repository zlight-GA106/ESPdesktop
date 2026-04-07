#include "ui_main.h"
#include "ui_font.h"     // ui_font_default()
#include <Arduino.h>

// ══════════════════════════════════════════════════════════════════
// ui_main.cpp — 三页 UI 系统实现
//   PAGE_HOME     : 主界面（时间 + 状态 + [设置][信息]）
//   PAGE_SETTINGS : 设置（WiFi设置 / 时间设置 / 系统信息 + [返回]）
//   PAGE_INFO     : 设备信息（名称/版本/芯片/LVGL版本 + [返回]）
//
// 布局规范:
//   ① 顶部标题栏 20px，深海军蓝背景
//   ② 内容区用 lv_obj_align / flex 布局，禁止写死像素坐标
//   ③ 字体全部通过 ui_font_default() 获取
// ══════════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────────
//  内部状态
// ─────────────────────────────────────────────────────────────────
static lv_obj_t *s_current_page = NULL;   ///< 当前显示的 screen
static lv_obj_t *s_page_to_del  = NULL;   ///< 即将异步删除的旧 screen

// ─────────────────────────────────────────────────────────────────
//  按钮回调前向声明（定义在文件底部，页面函数需先使用）
// ─────────────────────────────────────────────────────────────────
static void btn_settings_event(lv_event_t *e);
static void btn_info_event(lv_event_t *e);
static void btn_back_event(lv_event_t *e);

// ─────────────────────────────────────────────────────────────────
//  工具：创建顶部标题栏（深蓝 20px，居中白色文字）
// ─────────────────────────────────────────────────────────────────
static void _make_title_bar(lv_obj_t *scr, const char *title_text)
{
    lv_obj_t *bar = lv_obj_create(scr);
    lv_obj_set_size(bar, 128, 20);
    lv_obj_align(bar, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x000080), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(bar, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(bar, 2, LV_PART_MAIN);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl = lv_label_create(bar);
    lv_label_set_text(lbl, title_text);
    lv_obj_set_style_text_font(lbl, ui_font_default(), 0);
    lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
    lv_obj_center(lbl);
}

// ─────────────────────────────────────────────────────────────────
//  工具：创建文字按钮（蓝色圆角，白字居中）
// ─────────────────────────────────────────────────────────────────
static lv_obj_t* _make_button(lv_obj_t *parent,
                               const char *text,
                               int w,
                               lv_event_cb_t cb)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, w, 26);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_radius(btn, 4, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x0060C0), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_pad_all(btn, 2, LV_PART_MAIN);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, ui_font_default(), 0);
    lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
    lv_obj_center(lbl);
    return btn;
}

// ─────────────────────────────────────────────────────────────────
//  【主页】 — 主界面
//  布局（屏幕 128×160）:
//    [000000000 主界面 000000000]  ← 标题栏 y=0..20
//    [        时间: 12:00       ]  ← y≈38
//    [        状态: 正常         ]  ← y≈62
//    [                          ]
//    [ [设置]            [信息]  ]  ← 底部 y≈128
// ─────────────────────────────────────────────────────────────────
static lv_obj_t* ui_home_create(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x1A1A2E), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    // ── 标题栏："主界面" ──────────────────────────────────────────
    _make_title_bar(scr,
        "\xe4\xb8\xbb"   /* 主 */
        "\xe7\x95\x8c"   /* 界 */
        "\xe9\x9d\xa2"   /* 面 */);

    // ── 时间标签 ─────────────────────────────────────────────────
    lv_obj_t *lbl_time = lv_label_create(scr);
    lv_label_set_text(lbl_time,
        "\xe6\x97\xb6\xe9\x97\xb4"   /* 时间 */
        ": 12:00");
    lv_obj_set_style_text_font(lbl_time, ui_font_default(), 0);
    lv_obj_set_style_text_color(lbl_time, lv_color_white(), 0);
    lv_obj_align(lbl_time, LV_ALIGN_TOP_MID, 0, 38);

    // ── 状态标签 ─────────────────────────────────────────────────
    lv_obj_t *lbl_status = lv_label_create(scr);
    lv_label_set_text(lbl_status,
        "\xe7\x8a\xb6\xe6\x80\x81"   /* 状态 */
        ": "
        "\xe6\xad\xa3\xe5\xb8\xb8"   /* 正常 */);
    lv_obj_set_style_text_font(lbl_status, ui_font_default(), 0);
    lv_obj_set_style_text_color(lbl_status, lv_color_hex(0x00FF88), 0);
    lv_obj_align(lbl_status, LV_ALIGN_TOP_MID, 0, 62);

    // ── 底部按钮：[设置]  [信息] ──────────────────────────────────
    lv_obj_t *btn_s = _make_button(scr,
        "\xe8\xae\xbe\xe7\xbd\xae"   /* 设置 */,
        58, btn_settings_event);
    lv_obj_align(btn_s, LV_ALIGN_BOTTOM_LEFT, 4, -6);

    lv_obj_t *btn_i = _make_button(scr,
        "\xe4\xbf\xa1\xe6\x81\xaf"   /* 信息 */,
        58, btn_info_event);
    lv_obj_align(btn_i, LV_ALIGN_BOTTOM_RIGHT, -4, -6);

    return scr;
}

// ─────────────────────────────────────────────────────────────────
//  【设置页】— 设置
//  布局:
//    [000000000 设置  000000000]  ← 标题栏 y=0..20
//    [ WiFi设置                ]  ← flex 列表行
//    [ 时间设置                ]
//    [ 系统信息                ]
//    [          [返回]         ]  ← 底部
// ─────────────────────────────────────────────────────────────────
static lv_obj_t* ui_settings_create(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x1A1A2E), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    // ── 标题栏："设置" ────────────────────────────────────────────
    _make_title_bar(scr,
        "\xe8\xae\xbe\xe7\xbd\xae"   /* 设置 */);

    // ── 列表容器（flex 列，自动排列三行） ─────────────────────────
    lv_obj_t *list = lv_obj_create(scr);
    lv_obj_set_size(list, 120, 108);
    lv_obj_align(list, LV_ALIGN_TOP_MID, 0, 24);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(list, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(list, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(list, 6, LV_PART_MAIN);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(list, LV_OBJ_FLAG_SCROLLABLE);

    // ── 三个菜单项 ────────────────────────────────────────────────
    const char *items[] = {
        /* WiFi设置 */
        "WiFi"
        "\xe8\xae\xbe\xe7\xbd\xae",
        /* 时间设置 */
        "\xe6\x97\xb6\xe9\x97\xb4"
        "\xe8\xae\xbe\xe7\xbd\xae",
        /* 系统信息 */
        "\xe7\xb3\xbb\xe7\xbb\x9f"
        "\xe4\xbf\xa1\xe6\x81\xaf",
    };

    for (int i = 0; i < 3; i++) {
        lv_obj_t *row = lv_obj_create(list);
        lv_obj_set_size(row, lv_pct(100), 26);
        lv_obj_set_style_bg_color(row, lv_color_hex(0x0D3B6E), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_radius(row, 3, LV_PART_MAIN);
        lv_obj_set_style_border_width(row, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_left(row, 8, LV_PART_MAIN);
        lv_obj_set_style_pad_top(row, 4, LV_PART_MAIN);
        lv_obj_set_style_pad_right(row, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_bottom(row, 0, LV_PART_MAIN);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *lbl = lv_label_create(row);
        lv_label_set_text(lbl, items[i]);
        lv_obj_set_style_text_font(lbl, ui_font_default(), 0);
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
    }

    // ── 返回按钮 ──────────────────────────────────────────────────
    lv_obj_t *btn_back = _make_button(scr,
        "\xe8\xbf\x94\xe5\x9b\x9e"   /* 返回 */,
        60, btn_back_event);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -6);

    return scr;
}

// ─────────────────────────────────────────────────────────────────
//  【信息页】— 设备信息
//  布局:
//    [000000000 设备信息 0000000]  ← 标题栏 y=0..20
//    [ 名称: Demo设备            ]  ← flex 列
//    [ 版本: v1.0               ]
//    [ Chip: ESP32-S3           ]
//    [ LVGL: 8.3.11             ]
//    [          [返回]          ]  ← 底部
// ─────────────────────────────────────────────────────────────────
static lv_obj_t* ui_info_create(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x1A1A2E), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    // ── 标题栏："设备信息" ────────────────────────────────────────
    _make_title_bar(scr,
        "\xe8\xae\xbe\xe5\xa4\x87"   /* 设备 */
        "\xe4\xbf\xa1\xe6\x81\xaf"   /* 信息 */);

    // ── 内容容器（flex 列） ───────────────────────────────────────
    lv_obj_t *cont = lv_obj_create(scr);
    lv_obj_set_size(cont, 120, 100);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 24);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_left(cont, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_top(cont, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_right(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(cont, 8, LV_PART_MAIN);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    // ── 信息条目 ──────────────────────────────────────────────────
    const char *infos[] = {
        /* 名称: Demo设备 */
        "\xe5\x90\x8d\xe7\xa7\xb0"
        ": Demo"
        "\xe8\xae\xbe\xe5\xa4\x87",
        /* 版本: v1.0 */
        "\xe7\x89\x88\xe6\x9c\xac"
        ": v1.0",
        "Chip: ESP32-S3",
        "LVGL: 8.3.11",
    };

    for (int i = 0; i < 4; i++) {
        lv_obj_t *lbl = lv_label_create(cont);
        lv_label_set_text(lbl, infos[i]);
        lv_obj_set_style_text_font(lbl, ui_font_default(), 0);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xCCDDFF), 0);
    }

    // ── 返回按钮 ──────────────────────────────────────────────────
    lv_obj_t *btn_back = _make_button(scr,
        "\xe8\xbf\x94\xe5\x9b\x9e"   /* 返回 */,
        60, btn_back_event);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -6);

    return scr;
}

// ─────────────────────────────────────────────────────────────────
//  异步删除旧页面
//  [原因] 按钮回调是从 LVGL 渲染/事件循环中调用的。
//         若在回调中直接 lv_obj_del(旧屏幕)，而该屏幕正是
//         当前 LVGL 正在处理事件的 screen，会导致悬空指针崩溃。
//         使用 lv_async_call 将删除操作推迟到回调栈退出之后。
// ─────────────────────────────────────────────────────────────────
static void _delete_page_async(void *arg)
{
    (void)arg;
    if (s_page_to_del) {
        lv_obj_del(s_page_to_del);
        s_page_to_del = NULL;
    }
}

// ─────────────────────────────────────────────────────────────────
//  [公开 API] 页面切换
// ─────────────────────────────────────────────────────────────────
void ui_switch_page(ui_page_t page)
{
    // 1. 按需创建新页面 screen
    lv_obj_t *new_page = NULL;
    switch (page)
    {
        case PAGE_HOME:
            new_page = ui_home_create();
            break;
        case PAGE_SETTINGS:
            new_page = ui_settings_create();
            break;
        case PAGE_INFO:
            new_page = ui_info_create();
            break;
        default:
            new_page = ui_home_create();
            break;
    }

    // 2. 立即切换到新页面（无动画，嵌入式设备首选）
    lv_scr_load(new_page);

    // 3. 将旧页面挂入异步删除队列
    if (s_current_page != NULL) {
        s_page_to_del = s_current_page;
        lv_async_call(_delete_page_async, NULL);
    }

    // 4. 更新当前页引用
    s_current_page = new_page;

    Serial.printf("[UI-MAIN] page -> %d\n", (int)page);
}

// ─────────────────────────────────────────────────────────────────
//  按钮事件实现
// ─────────────────────────────────────────────────────────────────
static void btn_settings_event(lv_event_t *e)
{
    (void)e;
    ui_switch_page(PAGE_SETTINGS);
}

static void btn_info_event(lv_event_t *e)
{
    (void)e;
    ui_switch_page(PAGE_INFO);
}

static void btn_back_event(lv_event_t *e)
{
    (void)e;
    ui_switch_page(PAGE_HOME);
}

// ─────────────────────────────────────────────────────────────────
//  [公开 API] 启动入口
// ─────────────────────────────────────────────────────────────────
void ui_main_init(void)
{
    s_current_page = NULL;
    s_page_to_del  = NULL;
    ui_switch_page(PAGE_HOME);
}
