#include "ui_settings_menu_page.h"
#include "ui_i18n.h"
#include "../../app_data.h"
#include "ui_theme_pocketpc.h"
#include "ui_keypad.h"
#include <esp_task_wdt.h>

// ==========================================
// ⚙️ 设置主菜单页面实现 (Settings Menu Page)
// ==========================================

#define SETTINGS_ITEM_COUNT 6

// 每项对应的 i18n 键（与 settingsTargets 下标严格对齐）
static const UiStrKey s_labelKeys[SETTINGS_ITEM_COUNT] = {
    STR_SET_WIFI,
    STR_SET_BT,
    STR_SET_MQTT,
    STR_SET_BRIGHTNESS,
    STR_SET_VOLUME,
    STR_SET_LANGUAGE,
};

// 每项对应的目标页面 ID
static const int s_targets[SETTINGS_ITEM_COUNT] = {
    (int)WIFI_INFO_PAGE,
    (int)BLUETOOTH_INFO_PAGE,
    (int)MQTT_INFO_PAGE,
    (int)BRIGHTNESS_PAGE,
    (int)VOLUME_PAGE,
    (int)LANGUAGE_PAGE,
};

// ── 内部状态 ──────────────────────────────────────────────────────────────────
static uint8_t   s_selIdx = 0;
static lv_obj_t *s_items[SETTINGS_ITEM_COUNT]  = {};
static lv_obj_t *s_labels[SETTINGS_ITEM_COUNT] = {};

// ── 私有：刷新高亮样式（与 ui_menu_page 风格一致） ────────────────────────────
static void refreshStyles() {
    for (int i = 0; i < SETTINGS_ITEM_COUNT; i++) {
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

// ── 私有：在 parent 内构建内容 ────────────────────────────────────────────────
static void buildContent(lv_obj_t *parent) {
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(parent, LV_DIR_NONE);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(parent, 0, LV_PART_MAIN);

    const lv_coord_t rowX = 2;
    const lv_coord_t rowY = 2;
    const lv_coord_t rowW = 124;
    const lv_coord_t rowH = 18;
    const lv_coord_t rowGap = 1;

    for (int i = 0; i < SETTINGS_ITEM_COUNT; i++) {
        s_items[i] = lv_obj_create(parent);
        lv_obj_set_pos(s_items[i], rowX, rowY + i * (rowH + rowGap));
        lv_obj_set_size(s_items[i], rowW, rowH);
        lv_obj_clear_flag(s_items[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(s_items[i], 0, LV_PART_MAIN);
        lv_obj_set_style_border_width(s_items[i], 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(s_items[i], 0, LV_PART_MAIN);

        s_labels[i] = lv_label_create(s_items[i]);
        lv_label_set_text(s_labels[i], uiI18nGet(s_labelKeys[i]));
        lv_obj_set_style_text_font(s_labels[i], uiFontGetDefault(), LV_PART_MAIN);
        lv_obj_align(s_labels[i], LV_ALIGN_LEFT_MID, 4, 0);

        esp_task_wdt_reset();
    }

    refreshStyles();
    Serial.println("[SETTINGS] content ready");
}

// ── 公开接口 ───────────────────────────────────────────────────────────────────

lv_obj_t* uiSettingsMenuPageCreate() {
    return lv_obj_create(NULL);
}

void uiSettingsMenuPageSetSelected(uint8_t index) {
    if (index >= SETTINGS_ITEM_COUNT) index = SETTINGS_ITEM_COUNT - 1;
    s_selIdx = index;
    refreshStyles();
}

uint8_t uiSettingsMenuPageGetSelected() {
    return s_selIdx;
}

void uiSettingsMenuPageInit(lv_obj_t *parent) {
    s_selIdx = 0;
    buildContent(parent);
}

void uiSettingsMenuPageDeinit() {
    for (int i = 0; i < SETTINGS_ITEM_COUNT; i++) {
        s_items[i]  = nullptr;
        s_labels[i] = nullptr;
    }
}

void uiSettingsMenuPageUpdate(const SensorData &data, const SystemState &state) {
    (void)data; (void)state; // 静态菜单，无需持续刷新
}

void uiSettingsMenuPageRefreshLang() {
    for (int i = 0; i < SETTINGS_ITEM_COUNT; i++) {
        if (s_labels[i]) lv_label_set_text(s_labels[i], uiI18nGet(s_labelKeys[i]));
    }
    // 刷新后重新应用高亮样式（文字颜色可能受样式覆盖）
    refreshStyles();
}

int uiSettingsMenuPageHandleKey(uint32_t key) {
    if (key == UI_KEY_UP) {
        if (s_selIdx > 0) uiSettingsMenuPageSetSelected(s_selIdx - 1);
    } else if (key == UI_KEY_DOWN) {
        if (s_selIdx < SETTINGS_ITEM_COUNT - 1) uiSettingsMenuPageSetSelected(s_selIdx + 1);
    } else if (key == UI_KEY_OK) {
        return s_targets[s_selIdx];
    }
    return SETTINGS_MENU_PAGE;
}


// ── 以下为旧实现，已被上方新代码取代，此段在编译时会被忽略（通过 #if 0） ──
#if 0

// 内部状态：记录当前高亮的设置项索引 (加 static 保证退出再进时能记住上次选的位置)
static uint8_t selectedIndex = 0;

// 存储所有设置项的 LVGL 对象指针，方便快速刷样式
static lv_obj_t* settingsItems[SETTINGS_ITEM_COUNT];

// 设置项文字定义 (使用英文防止默认字体不支持中文导致乱码)
const char* settingsTexts[SETTINGS_ITEM_COUNT] = {
    "1. Wi-Fi Info",    // WiFi 信息
    "2. Bluetooth",     // 蓝牙信息
    "3. MQTT Cloud",    // MQTT 信息
    "4. Brightness",    // 屏幕亮度
    "5. Volume"         // 音量设置
};

// ==========================================
// [接口] 兼容层：创建基础对象
// ==========================================
lv_obj_t* uiSettingsMenuPageCreate() {
    return lv_obj_create(NULL);
}

// ==========================================
// [接口] 设置并刷新当前选中项的高亮样式
// ==========================================
void uiSettingsMenuPageSetSelected(uint8_t index) {
    if (index >= SETTINGS_ITEM_COUNT) index = SETTINGS_ITEM_COUNT - 1;
    selectedIndex = index;

    // 遍历所有设置项，更新焦点皮肤
    for (int i = 0; i < SETTINGS_ITEM_COUNT; i++) {
        if (settingsItems[i] == nullptr) continue;

        if (i == selectedIndex) {
            // [当前选中项高亮蓝] 赋予深蓝底白字的经典焦点样式
             lv_obj_add_style(settingsItems[i], &style_button_pressed, LV_PART_MAIN | LV_STATE_DEFAULT);
             lv_obj_remove_style(settingsItems[i], &style_button_normal, LV_PART_MAIN | LV_STATE_DEFAULT);
        } else {
            // 恢复普通样式
             lv_obj_add_style(settingsItems[i], &style_button_normal, LV_PART_MAIN | LV_STATE_DEFAULT);
             lv_obj_remove_style(settingsItems[i], &style_button_pressed, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }

    // 自动滚动，确保选中的项目不会掉出屏幕外
    if (settingsItems[selectedIndex] != nullptr) {
        lv_obj_scroll_to_view(settingsItems[selectedIndex], LV_ANIM_ON);
    }
}

// ==========================================
// [接口] 获取当前选中项
// ==========================================
uint8_t uiSettingsMenuPageGetSelected() {
    return selectedIndex;
}

// ==========================================
// [接口] 初始化设置菜单页面
// ==========================================
void uiSettingsMenuPageInit(lv_obj_t *parent) {
    // 弹性布局，垂直排列
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 动态生成设置项按钮
    for (int i = 0; i < SETTINGS_ITEM_COUNT; i++) {
        settingsItems[i] = lv_btn_create(parent);
        lv_obj_set_width(settingsItems[i], 120); 
        lv_obj_set_height(settingsItems[i], 24); 

        // 移除默认样式，应用 Pocket PC 皮肤
        lv_obj_remove_style_all(settingsItems[i]);
lv_obj_add_style(settingsItems[i], &style_button_pressed, LV_PART_MAIN | LV_STATE_PRESSED);

        // 创建文本
        lv_obj_t *label = lv_label_create(settingsItems[i]);
        lv_label_set_text(label, settingsTexts[i]);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 4, 0); 
    }

    // 初始化时强制刷新一次样式，点亮上次停留的光标
    uiSettingsMenuPageSetSelected(selectedIndex);
}

// ==========================================
// [接口] 页面数据刷新泵
// ==========================================
void uiSettingsMenuPageUpdate(const SensorData &data, const SystemState &state) {
    // 静态菜单页，无需持续刷新
}

// ==========================================
// [接口] 页面销毁清理
// ==========================================
void uiSettingsMenuPageDeinit() {
    // 安全置空指针，根节点销毁由 ui_manager 统管
    for (int i = 0; i < SETTINGS_ITEM_COUNT; i++) {
        settingsItems[i] = nullptr;
    }
}

// ==========================================
// [接口] 专属按键事件处理
// ==========================================
int uiSettingsMenuPageHandleKey(uint32_t key) {
    // [上下移动选中项时不播放 click.wav]
    // 焦点移动是静默的，符合老式掌机只在确认时发声的克制原则。
    
    if (key == UI_KEY_UP) {
        if (selectedIndex > 0) {
            uiSettingsMenuPageSetSelected(selectedIndex - 1);
        }
    } 
    else if (key == UI_KEY_DOWN) {
        if (selectedIndex < SETTINGS_ITEM_COUNT - 1) {
            uiSettingsMenuPageSetSelected(selectedIndex + 1);
        }
    } 
    else if (key == UI_KEY_OK) {
        // [进入下级页面时的 click.wav 由 ui_manager 控制]
        // 这里绝对不调用 audioPlayerPlayClick()。
        // 我们只负责向 ui_manager 报告：“用户想去这个页面了！”
        // ui_manager 收到非当前页面的 ID 后，会替我们播放 click.wav 并执行跳转压栈。
        
        switch (selectedIndex) {
            case 0: return WIFI_INFO_PAGE;
            case 1: return BLUETOOTH_INFO_PAGE;
            case 2: return MQTT_INFO_PAGE;
            case 3: return BRIGHTNESS_PAGE;
            case 4: return VOLUME_PAGE;
        }
    }

    // 未触发跳转则返回当前页 ID
    return SETTINGS_MENU_PAGE;
}
#endif // end of old implementation