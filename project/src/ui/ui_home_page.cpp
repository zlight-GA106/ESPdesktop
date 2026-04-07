#include <Arduino.h>
#include "ui_home_page.h"
#include "ui_frame.h"          // uiFrameCreate
#include "ui_font.h"           // uiFontGetDefault()
#include "../../app_data.h"    // SensorData, SystemState, HOME_PAGE enum
#include <esp_task_wdt.h>
#include <stdio.h>

// ==========================================
// 首页内部控件指针（静态，update 函数复用）
// ==========================================
static lv_obj_t *timeLabel   = nullptr;  // 大号时间显示
static lv_obj_t *lunarLabel  = nullptr;  // 农历占位
static lv_obj_t *sensorLabel = nullptr;  // 温湿度概览

// 独立测试模式下的 frame 句柄
static UiFrame s_frame;

// ==========================================
// 私有：将所有内容控件填充到指定父容器
// [设计]：buildContent 与"谁创建了根节点"无关，
//         无论是 uiHomePageCreate（独立模式）
//         还是 uiHomePageInit（ui_manager 托管模式）都调用它。
// ==========================================
static void buildContent(lv_obj_t *parent) {
    // HOME 不需要滚动，禁用后可避免无意义的布局/滚动计算。
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(parent, LV_DIR_NONE);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    esp_task_wdt_reset();

    // ---- 1. 时间标签 ----
    // 直接使用带背景的 label，避免额外容器对象和布局计算。
    timeLabel = lv_label_create(parent);
    lv_label_set_text(timeLabel, "07:58");
    lv_obj_set_width(timeLabel, 84);
    lv_obj_set_style_text_align(timeLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_pad_top(timeLabel, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(timeLabel, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_left(timeLabel, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_right(timeLabel, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(timeLabel, lv_color_hex(0x000080), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(timeLabel, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(timeLabel, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(timeLabel, 0, LV_PART_MAIN);
    lv_obj_set_style_text_font(timeLabel, ui_font_default(), LV_PART_MAIN);
    lv_obj_set_style_text_color(timeLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(timeLabel, LV_ALIGN_TOP_MID, 0, 10);
    esp_task_wdt_reset();

    // ---- 2. 农历占位 ----
    // [视觉]：深灰色小字，区别于时间主体
    lunarLabel = lv_label_create(parent);
    lv_label_set_text(lunarLabel, "Date: Jun 3");
    lv_obj_set_style_text_font(lunarLabel, ui_font_default(), LV_PART_MAIN);
    lv_obj_set_style_text_color(lunarLabel, lv_color_hex(0x404040), LV_PART_MAIN);
    lv_obj_align(lunarLabel, LV_ALIGN_TOP_MID, 0, 50);
    esp_task_wdt_reset();

    // ---- 3. 温湿度概览 ----
    // [视觉]：黑色小字，紧凑排布
    sensorLabel = lv_label_create(parent);
    lv_label_set_text(sensorLabel, "25.3C / 48%");
    lv_obj_set_style_text_font(sensorLabel, ui_font_default(), LV_PART_MAIN);
    lv_obj_set_style_text_color(sensorLabel, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_align(sensorLabel, LV_ALIGN_TOP_MID, 0, 74);
    esp_task_wdt_reset();

    Serial.println("[HOME] content ready");
}

// ==========================================
// [接口] 独立/测试模式：自建 Frame 并返回根节点
// ==========================================
lv_obj_t* uiHomePageCreate() {
    s_frame = uiFrameCreate("Home");
    buildContent(s_frame.content);
    return s_frame.root;
}

// ==========================================
// [接口] ui_manager 托管模式：在已有 content 区创建控件
// ==========================================
void uiHomePageInit(lv_obj_t *parent) {
    buildContent(parent);
}

// ==========================================
// [接口] 数据刷新泵（create/init 后调用）
// [规则]：只修改文本，绝不在此创建新控件
// ==========================================
void uiHomePageUpdate(const SensorData &data, const SystemState &state) {
    if (timeLabel) {
        lv_label_set_text_fmt(timeLabel, "%02d:%02d",
                              state.hour, state.minute);
    }
    if (lunarLabel) {
        lv_label_set_text_fmt(lunarLabel, "Date: %04d-%02d-%02d",
                              state.year, state.month, state.day);
    }
    if (sensorLabel) {
        lv_label_set_text_fmt(sensorLabel, "%.1fC / %.0f%%",
                              data.temperature, data.humidity);
    }
}

// ==========================================
// [接口] 销毁清理（置空悬空指针）
// ==========================================
void uiHomePageDeinit() {
    timeLabel   = nullptr;
    lunarLabel  = nullptr;
    sensorLabel = nullptr;
}

// ==========================================
// [接口] 按键路由（首页不处理内部导航）
// ==========================================
int uiHomePageHandleKey(uint32_t key) {
    (void)key;
    return (int)HOME_PAGE;
}