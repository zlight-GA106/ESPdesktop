#include "ui_language_page.h"
#include "ui_i18n.h"
#include "ui_theme_pocketpc.h"
#include "ui_keypad.h"
#include "../../app_data.h"

// ─── 内部状态 ──────────────────────────────────────────────────────────────────
// 选项只有两个，索引对应 UiLang 枚举值
#define LANG_ITEM_COUNT 2

static uint8_t   s_selIdx    = 0;          // 当前高亮索引（0=ZH, 1=EN）
static bool      s_changed   = false;      // OK 按下后置 true，被消费后清零
static lv_obj_t *s_items[LANG_ITEM_COUNT]  = {};   // 行容器
static lv_obj_t *s_labels[LANG_ITEM_COUNT] = {};   // 行文字标签

// ─── 私有：刷新高亮样式 ────────────────────────────────────────────────────────
static void refreshStyles() {
    for (int i = 0; i < LANG_ITEM_COUNT; i++) {
        if (!s_items[i]) continue;

        lv_obj_remove_style(s_items[i], &style_list_selected, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_remove_style(s_items[i], &style_list_item,     LV_PART_MAIN | LV_STATE_DEFAULT);

        if (i == s_selIdx) {
            lv_obj_add_style(s_items[i], &style_list_selected, LV_PART_MAIN | LV_STATE_DEFAULT);
            if (s_labels[i]) lv_obj_set_style_text_color(s_labels[i],
                lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        } else {
            lv_obj_add_style(s_items[i], &style_list_item, LV_PART_MAIN | LV_STATE_DEFAULT);
            if (s_labels[i]) lv_obj_set_style_text_color(s_labels[i],
                lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
}

// ─── 私有：建立选项列表 ────────────────────────────────────────────────────────
static void buildContent(lv_obj_t *parent) {
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START,
                                   LV_FLEX_ALIGN_CENTER,
                                   LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scroll_dir(parent, LV_DIR_NONE);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);

    static const UiStrKey labelKeys[LANG_ITEM_COUNT] = {
        STR_LANG_ZH_LABEL,
        STR_LANG_EN_LABEL
    };

    for (int i = 0; i < LANG_ITEM_COUNT; i++) {
        s_items[i] = lv_obj_create(parent);
        lv_obj_set_width(s_items[i], lv_pct(100));
        lv_obj_set_height(s_items[i], 24);
        lv_obj_clear_flag(s_items[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(s_items[i], 0, LV_PART_MAIN);
        lv_obj_set_style_border_width(s_items[i], 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(s_items[i], 0, LV_PART_MAIN);

        s_labels[i] = lv_label_create(s_items[i]);
        lv_label_set_text(s_labels[i], uiI18nGet(labelKeys[i]));
        lv_obj_set_style_text_font(s_labels[i], uiFontGetDefault(), LV_PART_MAIN);
        lv_obj_align(s_labels[i], LV_ALIGN_LEFT_MID, 4, 0);
    }

    // 初次加载：高亮与当前激活语言对齐
    s_selIdx = (uint8_t)uiI18nGetLanguage();
    if (s_selIdx >= LANG_ITEM_COUNT) s_selIdx = 0;
    refreshStyles();
}

// ─── 公开接口实现 ──────────────────────────────────────────────────────────────

void uiLanguagePageInit(lv_obj_t *parent) {
    s_changed = false;
    buildContent(parent);
}

void uiLanguagePageDeinit() {
    for (int i = 0; i < LANG_ITEM_COUNT; i++) {
        s_items[i] = nullptr;
        s_labels[i] = nullptr;
    }
    s_changed = false;
}

void uiLanguagePageUpdate(const SensorData &data, const SystemState &state) {
    (void)data; (void)state;
}

void uiLanguagePageRefreshLang() {
    static const UiStrKey labelKeys[LANG_ITEM_COUNT] = {
        STR_LANG_ZH_LABEL,
        STR_LANG_EN_LABEL
    };
    for (int i = 0; i < LANG_ITEM_COUNT; i++) {
        if (s_labels[i]) lv_label_set_text(s_labels[i], uiI18nGet(labelKeys[i]));
    }
    // 高亮同步到已生效的语言
    s_selIdx = (uint8_t)uiI18nGetLanguage();
    if (s_selIdx >= LANG_ITEM_COUNT) s_selIdx = 0;
    refreshStyles();
}

bool uiLanguagePageConsumeChanged() {
    if (s_changed) {
        s_changed = false;
        return true;
    }
    return false;
}

int uiLanguagePageHandleKey(uint32_t key) {
    if (key == UI_KEY_UP) {
        if (s_selIdx > 0) {
            s_selIdx--;
            refreshStyles();
        }
    } else if (key == UI_KEY_DOWN) {
        if (s_selIdx < LANG_ITEM_COUNT - 1) {
            s_selIdx++;
            refreshStyles();
        }
    } else if (key == UI_KEY_OK) {
        // 应用语言：直接更新 i18n 模块，标记 changed
        uiI18nSetLanguage((UiLang)s_selIdx);
        s_changed = true;
        Serial.printf("[LangPage] OK -> lang=%d\n", s_selIdx);
    }
    return LANGUAGE_PAGE;
}
