#include <Arduino.h>
#include "ui_menu_page.h"
#include "ui_frame.h"           // uiFrameCreate
#include "ui_font.h"            // uiFontGetDefault()
#include "ui_theme_pocketpc.h"  // style_list_item, style_list_selected
#include "../../app_data.h"     // SensorData, SystemState, UiPageId enum
#include <esp_task_wdt.h>
#include <stdio.h>

// ==========================================
// 菜单项定义
// ==========================================
#define MENU_ITEM_COUNT 6

static const char *menuTexts[MENU_ITEM_COUNT] = {
    "1. Sensors",       // 传感器
    "2. Settings",      // 设置
    "3. Web Radio",     // 网络电台
    "4. AI Assistant",  // 人工智能助手
    "5. Dino Game",     // 小恐龙游戏
    "6. About Device",  // 关于本机
};

// 每个菜单项对应的目标页面 ID（和 UiPageId 枚举对齐）
static const int menuTargets[MENU_ITEM_COUNT] = {
    (int)SENSOR_PAGE,
    (int)SETTINGS_MENU_PAGE,
    (int)RADIO_PAGE,
    (int)AI_PAGE,
    (int)DINO_PAGE,
    (int)ABOUT_PAGE,
};

// ==========================================
// 内部状态
// ==========================================
static uint8_t    selectedIndex = 0;
static lv_obj_t  *menuItems[MENU_ITEM_COUNT];   // 每行 lv_obj_t（容器）
static lv_obj_t  *menuLabels[MENU_ITEM_COUNT];  // 每行的文字 label

// 独立模式下的 frame 句柄
static UiFrame s_frame;

// ==========================================
// 私有：将高亮样式刷新到所有行
// ==========================================
static void refreshStyles() {
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        if (menuItems[i] == nullptr) continue;

        // 清除两种列表样式后重新设置，避免叠加
        lv_obj_remove_style(menuItems[i], &style_list_selected, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_remove_style(menuItems[i], &style_list_item,     LV_PART_MAIN | LV_STATE_DEFAULT);

        if (i == selectedIndex) {
            lv_obj_add_style(menuItems[i], &style_list_selected, LV_PART_MAIN | LV_STATE_DEFAULT);
            if (menuLabels[i]) {
                lv_obj_set_style_text_color(menuLabels[i],
                    lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            }
        } else {
            lv_obj_add_style(menuItems[i], &style_list_item, LV_PART_MAIN | LV_STATE_DEFAULT);
            if (menuLabels[i]) {
                lv_obj_set_style_text_color(menuLabels[i],
                    lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            }
        }
    }
}

// ==========================================
// 私有：在 parent 内创建所有菜单行
// ==========================================
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

    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        menuItems[i] = lv_obj_create(parent);
        lv_obj_set_pos(menuItems[i], rowX, rowY + i * (rowH + rowGap));
        lv_obj_set_size(menuItems[i], rowW, rowH);
        lv_obj_clear_flag(menuItems[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(menuItems[i], 0, LV_PART_MAIN);
        lv_obj_set_style_border_width(menuItems[i], 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(menuItems[i], 0, LV_PART_MAIN);

        // 行内文字 label
        menuLabels[i] = lv_label_create(menuItems[i]);
        lv_label_set_text(menuLabels[i], menuTexts[i]);
        lv_obj_set_style_text_font(menuLabels[i],
            uiFontGetDefault(), LV_PART_MAIN);
        lv_obj_align(menuLabels[i], LV_ALIGN_LEFT_MID, 4, 0);

        esp_task_wdt_reset();
    }

    refreshStyles();
    Serial.println("[MENU] content ready");
}

// ==========================================
// [接口] 独立/测试模式：自建 Frame 并返回根节点
// ==========================================
lv_obj_t *uiMenuPageCreate() {
    s_frame = uiFrameCreate("Menu");
    buildContent(s_frame.content);
    return s_frame.root;
}

// ==========================================
// [接口] 设置选中项并刷新高亮
// ==========================================
void uiMenuPageSetSelected(uint8_t index) {
    if (index >= MENU_ITEM_COUNT) index = MENU_ITEM_COUNT - 1;
    selectedIndex = index;
    refreshStyles();
}

// ==========================================
// [接口] 获取选中项
// ==========================================
uint8_t uiMenuPageGetSelected() {
    return selectedIndex;
}

// ==========================================
// [接口] ui_manager 托管模式：在已有 content 创建菜单
// ==========================================
void uiMenuPageInit(lv_obj_t *parent) {
    buildContent(parent);
}

// ==========================================
// [接口] 数据刷新�（菜单为静态内容，留空）
// ==========================================
void uiMenuPageUpdate(const SensorData &data, const SystemState &state) {
    (void)data; (void)state;
}

// ==========================================
// [接口] 销毁清理
// ==========================================
void uiMenuPageDeinit() {
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        menuItems[i]  = nullptr;
        menuLabels[i] = nullptr;
    }
}

// ==========================================
// [接口] 按键处理（不引用 ui_manager/ui_keypad）
// 调用方通过返回值决定页面跳转，本函数不做导航
// ==========================================
int uiMenuPageHandleKey(uint32_t key) {
    // 方向键常量直接用数值，避免依赖 ui_keypad.h
    // 上=1, 下=2, OK=5（与 config.h 对齐，由调用方传入）
    const uint32_t KEY_UP   = 1;
    const uint32_t KEY_DOWN = 2;
    const uint32_t KEY_OK   = 5;

    if (key == KEY_UP && selectedIndex > 0) {
        uiMenuPageSetSelected(selectedIndex - 1);
    } else if (key == KEY_DOWN && selectedIndex < MENU_ITEM_COUNT - 1) {
        uiMenuPageSetSelected(selectedIndex + 1);
    } else if (key == KEY_OK) {
        return menuTargets[selectedIndex];
    }

    return (int)MENU_PAGE;
}
