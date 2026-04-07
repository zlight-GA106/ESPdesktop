#include "ui_brightness_page.h"
#include "../../app_data.h"
#include "ui_i18n.h"
#include "ui_theme_pocketpc.h"
#include "ui_keypad.h"

// ==========================================
// 💡 屏幕亮度调节页面实现 (Brightness Page)
// ==========================================

// 内部静态状态
static int currentBrightness = 50; // 当前 UI 暂存的亮度百分比 (0-100)

// 内部静态控件指针
static lv_obj_t *brightnessLabel = nullptr;
static lv_obj_t *brightnessBar   = nullptr;
static lv_obj_t *hintLabel       = nullptr;  // 操作提示：↑+  ↓-  ←返

// ==========================================
// [内部] 构建提示文字
// ==========================================
static void buildHint() {
    if (!hintLabel) return;
    char hint[40];
    // 使用 LVGL 内置符号（不依赖 CJK），"Back" 部分通过 i18n 取
    snprintf(hint, sizeof(hint), "%s+  %s-  %s%s",
             LV_SYMBOL_UP, LV_SYMBOL_DOWN, LV_SYMBOL_LEFT,
             uiI18nGet(STR_HINT_BACK));
    lv_label_set_text(hintLabel, hint);
}

// ==========================================
// [接口] 兼容层：单独创建页面对象
// ==========================================
lv_obj_t* uiBrightnessPageCreate() {
    return lv_obj_create(NULL);
}

// ==========================================
// [接口] 初始化亮度调节页面
// ==========================================
void uiBrightnessPageInit(lv_obj_t *parent) {
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(parent, 12, 0);

    // 1. 操作提示 (↑+  ↓-  ←返)
    hintLabel = lv_label_create(parent);
    buildHint();

    // 2. 当前亮度值文本
    brightnessLabel = lv_label_create(parent);
    {
        char buf[32];
        snprintf(buf, sizeof(buf), uiI18nGet(STR_BRIGHTNESS_FMT), currentBrightness);
        lv_label_set_text(brightnessLabel, buf);
    }

    // 3. 亮度进度条 (Bar)
    brightnessBar = lv_bar_create(parent);
    lv_obj_set_size(brightnessBar, 110, 20);
    lv_bar_set_range(brightnessBar, 0, 100);
    lv_bar_set_value(brightnessBar, currentBrightness, LV_ANIM_OFF);

    lv_obj_set_style_radius(brightnessBar, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(brightnessBar, 0, LV_PART_INDICATOR);
    lv_obj_set_style_border_width(brightnessBar, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(brightnessBar, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(brightnessBar, lv_color_hex(0x000080), LV_PART_INDICATOR);
}

// ==========================================
// [接口] 独立更新亮度界面的数值
// ==========================================
void uiBrightnessPageUpdate(int brightnessValue) {
    if (brightnessValue < 0) brightnessValue = 0;
    if (brightnessValue > 100) brightnessValue = 100;
    currentBrightness = brightnessValue;

    if (brightnessLabel) {
        char buf[32];
        snprintf(buf, sizeof(buf), uiI18nGet(STR_BRIGHTNESS_FMT), currentBrightness);
        lv_label_set_text(brightnessLabel, buf);
    }
    if (brightnessBar) {
        lv_bar_set_value(brightnessBar, currentBrightness, LV_ANIM_ON);
    }
}

// ==========================================
// [接口] 页面数据刷新泵 (大 Update)
// ==========================================
void uiBrightnessPageUpdate(const SensorData &data, const SystemState &state) {
    // 硬件背光接入后，可取消下面注释同步状态
    // uiBrightnessPageUpdate(state.brightness);
    (void)data; (void)state;
}

// ==========================================
// [接口] 步进调节亮度界面的数值
// ==========================================
void uiBrightnessPageAdjust(int delta) {
    uiBrightnessPageUpdate(currentBrightness + delta);
    // 真实 PWM 背光调节: ledcWrite(TFT_BL_CH, map(currentBrightness, 0, 100, 0, 255));
}

// ==========================================
// [接口] 页面销毁清理
// ==========================================
void uiBrightnessPageDeinit() {
    brightnessLabel = nullptr;
    brightnessBar   = nullptr;
    hintLabel       = nullptr;
}

// ==========================================
// [接口] 语言切换后刷新标签文字
// ==========================================
void uiBrightnessPageRefreshLang() {
    buildHint();
    if (brightnessLabel) {
        char buf[32];
        snprintf(buf, sizeof(buf), uiI18nGet(STR_BRIGHTNESS_FMT), currentBrightness);
        lv_label_set_text(brightnessLabel, buf);
    }
}

// ==========================================
// [接口] 专属按键事件处理
// 交互规则：
//   ↑ (UP)   = 亮度 +10
//   ↓ (DOWN) = 亮度 -10
//   ← (LEFT) = 已被 ui_manager 全局拦截为"返回"，此处不处理
//   → (RIGHT) = 无动作
//   OK        = 无动作（当前阶段）
// ==========================================
int uiBrightnessPageHandleKey(uint32_t key) {
    if (key == UI_KEY_UP) {
        uiBrightnessPageAdjust(10);   // 每次 +10%
    }
    else if (key == UI_KEY_DOWN) {
        uiBrightnessPageAdjust(-10);  // 每次 -10%
    }
    // UI_KEY_LEFT 由 ui_manager 全局拦截，此函数永远不会收到 LEFT
    // UI_KEY_RIGHT 无动作
    return BRIGHTNESS_PAGE;
}

