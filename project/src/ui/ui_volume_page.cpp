#include "ui_volume_page.h"
#include "../../app_data.h"
#include "ui_i18n.h"
#include "ui_theme_pocketpc.h"
#include "ui_keypad.h"

// ==========================================
// 🔊 系统音量调节页面实现 (Volume Page)
// ==========================================

// 内部静态状态
static int currentVolume = 50; // 当前 UI 暂存的音量百分比 (0-100)

// 内部静态控件指针
static lv_obj_t *volumeLabel  = nullptr;
static lv_obj_t *volumeBar    = nullptr;
static lv_obj_t *hintLabel    = nullptr;  // 操作提示：↑+  ↓-  ←返

// ==========================================
// [内部] 构建提示文字
// ==========================================
static void buildHint() {
    if (!hintLabel) return;
    char hint[40];
    snprintf(hint, sizeof(hint), "%s+  %s-  %s%s",
             LV_SYMBOL_UP, LV_SYMBOL_DOWN, LV_SYMBOL_LEFT,
             uiI18nGet(STR_HINT_BACK));
    lv_label_set_text(hintLabel, hint);
}

// ==========================================
// [接口] 兼容层：单独创建页面对象
// ==========================================
lv_obj_t* uiVolumePageCreate() {
    return lv_obj_create(NULL);
}

// ==========================================
// [接口] 初始化音量调节页面
// ==========================================
void uiVolumePageInit(lv_obj_t *parent) {
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(parent, 12, 0);

    // 1. 操作提示 (↑+  ↓-  ←返)
    hintLabel = lv_label_create(parent);
    buildHint();

    // 2. 当前音量值文本
    volumeLabel = lv_label_create(parent);
    {
        char buf[32];
        snprintf(buf, sizeof(buf), uiI18nGet(STR_VOLUME_FMT), currentVolume);
        lv_label_set_text(volumeLabel, buf);
    }

    // 3. 音量进度条 (Bar)
    volumeBar = lv_bar_create(parent);
    lv_obj_set_size(volumeBar, 110, 20);
    lv_bar_set_range(volumeBar, 0, 100);
    lv_bar_set_value(volumeBar, currentVolume, LV_ANIM_OFF);

    lv_obj_set_style_radius(volumeBar, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(volumeBar, 0, LV_PART_INDICATOR);
    lv_obj_set_style_border_width(volumeBar, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(volumeBar, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(volumeBar, lv_color_hex(0x000080), LV_PART_INDICATOR);
}

// ==========================================
// [接口] 独立更新音量界面的数值 (小 Update)
// ==========================================
void uiVolumePageUpdate(int volumeValue) {
    if (volumeValue < 0) volumeValue = 0;
    if (volumeValue > 100) volumeValue = 100;
    currentVolume = volumeValue;

    if (volumeLabel) {
        char buf[32];
        snprintf(buf, sizeof(buf), uiI18nGet(STR_VOLUME_FMT), currentVolume);
        lv_label_set_text(volumeLabel, buf);
    }
    if (volumeBar) {
        lv_bar_set_value(volumeBar, currentVolume, LV_ANIM_ON);
    }
}

// ==========================================
// [接口] 页面数据刷新泵 (大 Update)
// ==========================================
void uiVolumePageUpdate(const SensorData &data, const SystemState &state) {
    (void)data; (void)state;
    // uiVolumePageUpdate(state.volume);  // 打通底层后取消注释
}

// ==========================================
// [接口] 步进调节音量界面的数值
// ==========================================
void uiVolumePageAdjust(int delta) {
    uiVolumePageUpdate(currentVolume + delta);
    // audioPlayerSetVolume(currentVolume);  // 真实 I2S 音量调节
}

// ==========================================
// [接口] 页面销毁清理
// ==========================================
void uiVolumePageDeinit() {
    volumeLabel = nullptr;
    volumeBar   = nullptr;
    hintLabel   = nullptr;
}

// ==========================================
// [接口] 语言切换后刷新标签文字
// ==========================================
void uiVolumePageRefreshLang() {
    buildHint();
    if (volumeLabel) {
        char buf[32];
        snprintf(buf, sizeof(buf), uiI18nGet(STR_VOLUME_FMT), currentVolume);
        lv_label_set_text(volumeLabel, buf);
    }
}

// ==========================================
// [接口] 专属按键事件处理
// 交互规则：
//   ↑ (UP)   = 音量 +5
//   ↓ (DOWN) = 音量 -5
//   ← (LEFT) = 已被 ui_manager 全局拦截为"返回"，此处不处理
//   → (RIGHT) = 无动作
// ==========================================
int uiVolumePageHandleKey(uint32_t key) {
    if (key == UI_KEY_UP) {
        uiVolumePageAdjust(5);    // 每次 +5%
    }
    else if (key == UI_KEY_DOWN) {
        uiVolumePageAdjust(-5);   // 每次 -5%
    }
    // UI_KEY_LEFT 由 ui_manager 全局拦截，此函数永远不会收到 LEFT
    // UI_KEY_RIGHT 无动作
    return VOLUME_PAGE;
}