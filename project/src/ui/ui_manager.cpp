#include "ui_manager.h"
#include "ui_i18n.h"
#include "../../app_data.h"
#include "ui_frame.h"
#include "ui_keypad.h"
#include "ui_home_page.h"
#include "ui_menu_page.h"
#include "ui_settings_menu_page.h"
#include "ui_wifi_info_page.h"
#include "ui_bluetooth_info_page.h"
#include "ui_mqtt_info_page.h"
#include "ui_brightness_page.h"
#include "ui_volume_page.h"
#include "ui_language_page.h"
#include <esp_task_wdt.h>  // esp_task_wdt_reset()

// =====================================================
// UI Manager — UI-7  多页导航栈 + 双语言
//
// 设计原则: "lazy-build, never lv_obj_del"
//   启动时只建立 HOME，其他页面首次进入时再创建并缓存。
//   导航只调用 lv_scr_load()，绝不销毁任何已建屏幕。
//   这样可显著降低开机阶段的 CPU/内存峰值，并规避 WDT 重启。
//
// 导航栈：最深 8 层（HOME→MENU→SETTINGS→子页，足够）
// =====================================================

#define PAGE_POOL_SIZE  15   // UiPageId 最大值 + 1（PLACEHOLDER_PAGE=14）
#define NAV_STACK_SIZE   8

// ── 每个预建页的句柄 ──────────────────────────────────────────────────────────
static lv_obj_t *s_roots[PAGE_POOL_SIZE]      = {};  // 每个页面的 lv_scr root
static lv_obj_t *s_statusLbls[PAGE_POOL_SIZE] = {};  // 各页面 frame.statusLabel
static lv_obj_t *s_titleLbls[PAGE_POOL_SIZE]  = {};  // 各页面 frame.titleLabel

// ── 导航状态 ─────────────────────────────────────────────────────────────────
static UiPageId s_page                = HOME_PAGE;
static int      s_navStack[NAV_STACK_SIZE] = {};
static int      s_navDepth            = 0;

// ── 内部工具 ─────────────────────────────────────────────────────────────────
static void cachePage(UiPageId id, const UiFrame &f) {
    s_roots[id]      = f.root;
    s_statusLbls[id] = f.statusLabel;
    s_titleLbls[id]  = f.titleLabel;
}

static const char* pageName(UiPageId id) {
    switch (id) {
        case HOME_PAGE:          return "HOME";
        case MENU_PAGE:          return "MENU";
        case SETTINGS_MENU_PAGE: return "SETTINGS";
        case WIFI_INFO_PAGE:     return "WIFI";
        case BLUETOOTH_INFO_PAGE:return "BT";
        case MQTT_INFO_PAGE:     return "MQTT";
        case BRIGHTNESS_PAGE:    return "BRIGHTNESS";
        case VOLUME_PAGE:        return "VOLUME";
        case LANGUAGE_PAGE:      return "LANGUAGE";
        default:                 return "???";
    }
}

static bool buildPage(UiPageId id) {
    if (id < 0 || id >= PAGE_POOL_SIZE) return false;
    if (s_roots[id] != nullptr) return true;

    UiFrame f;
    switch (id) {
        case HOME_PAGE:
            f = uiFrameCreate(uiI18nGet(STR_TITLE_HOME));
            uiHomePageInit(f.content);
            cachePage(HOME_PAGE, f);
            Serial.printf("[UI-7] HOME root=%p\n", (void*)f.root);
            break;

        case MENU_PAGE:
            f = uiFrameCreate(uiI18nGet(STR_TITLE_MENU));
            uiMenuPageInit(f.content);
            uiMenuPageSetSelected(0);
            cachePage(MENU_PAGE, f);
            Serial.printf("[UI-7] MENU root=%p\n", (void*)f.root);
            break;

        case SETTINGS_MENU_PAGE:
            f = uiFrameCreate(uiI18nGet(STR_TITLE_SETTINGS));
            uiSettingsMenuPageInit(f.content);
            cachePage(SETTINGS_MENU_PAGE, f);
            Serial.printf("[UI-7] SETTINGS root=%p\n", (void*)f.root);
            break;

        case WIFI_INFO_PAGE:
            f = uiFrameCreate(uiI18nGet(STR_TITLE_WIFI));
            uiWifiInfoPageInit(f.content);
            cachePage(WIFI_INFO_PAGE, f);
            Serial.printf("[UI-7] WIFI root=%p\n", (void*)f.root);
            break;

        case BLUETOOTH_INFO_PAGE:
            f = uiFrameCreate(uiI18nGet(STR_TITLE_BLUETOOTH));
            uiBluetoothInfoPageInit(f.content);
            cachePage(BLUETOOTH_INFO_PAGE, f);
            Serial.printf("[UI-7] BT root=%p\n", (void*)f.root);
            break;

        case MQTT_INFO_PAGE:
            f = uiFrameCreate(uiI18nGet(STR_TITLE_MQTT));
            uiMqttInfoPageInit(f.content);
            cachePage(MQTT_INFO_PAGE, f);
            Serial.printf("[UI-7] MQTT root=%p\n", (void*)f.root);
            break;

        case BRIGHTNESS_PAGE:
            f = uiFrameCreate(uiI18nGet(STR_TITLE_BRIGHTNESS));
            uiBrightnessPageInit(f.content);
            cachePage(BRIGHTNESS_PAGE, f);
            Serial.printf("[UI-7] BRIGHTNESS root=%p\n", (void*)f.root);
            break;

        case VOLUME_PAGE:
            f = uiFrameCreate(uiI18nGet(STR_TITLE_VOLUME));
            uiVolumePageInit(f.content);
            cachePage(VOLUME_PAGE, f);
            Serial.printf("[UI-7] VOLUME root=%p\n", (void*)f.root);
            break;

        case LANGUAGE_PAGE:
            f = uiFrameCreate(uiI18nGet(STR_TITLE_LANGUAGE));
            uiLanguagePageInit(f.content);
            cachePage(LANGUAGE_PAGE, f);
            Serial.printf("[UI-7] LANGUAGE root=%p\n", (void*)f.root);
            break;

        default:
            Serial.printf("[UI-7] buildPage: unsupported id=%d\n", (int)id);
            return false;
    }

    esp_task_wdt_reset();
    delay(10);
    return true;
}

// ── 状态栏格式化（使用 i18n 前缀，支持中英切换） ────────────────────────────
static void pushStatus(UiPageId id, const SystemState &state) {
    if (!s_statusLbls[id]) return;
    char buf[32];
    snprintf(buf, sizeof(buf), "%s%s%s%s",
             uiI18nGet(STR_STATUS_WIFI_PREFIX),
             state.wifiConnected ? uiI18nGet(STR_STATUS_WIFI_OK)
                                 : uiI18nGet(STR_STATUS_WIFI_DASH),
             uiI18nGet(STR_STATUS_AI_PREFIX),
             state.aiBusy        ? uiI18nGet(STR_STATUS_AI_BUSY)
                                 : uiI18nGet(STR_STATUS_AI_IDLE));
    lv_label_set_text(s_statusLbls[id], buf);
}

// ─────────────────────────────────────────────────────────────────────────────
// [API] Begin: 初始化管理器，仅建立 HOME 并加载
// ─────────────────────────────────────────────────────────────────────────────
void uiManagerBegin() {
    uiI18nInit(LANG_ZH_CN);   // 默认中文
    memset(s_roots,      0, sizeof(s_roots));
    memset(s_statusLbls, 0, sizeof(s_statusLbls));
    memset(s_titleLbls,  0, sizeof(s_titleLbls));
    s_navDepth = 0;
    s_page     = HOME_PAGE;

    Serial.println("[UI-7] building HOME (others lazy-built)...");
    buildPage(HOME_PAGE);

    lv_scr_load(s_roots[HOME_PAGE]);
    Serial.println("[UI-7] manager ready, HOME loaded");
}

// ─────────────────────────────────────────────────────────────────────────────
// [API] Update: 推送数据到当前活跃页面
// ─────────────────────────────────────────────────────────────────────────────
void uiManagerUpdate(const SensorData &data, const SystemState &state) {
    pushStatus(s_page, state);

    switch (s_page) {
        case HOME_PAGE:          uiHomePageUpdate(data, state);          break;
        case MENU_PAGE:          uiMenuPageUpdate(data, state);          break;
        case WIFI_INFO_PAGE:     uiWifiInfoPageUpdate(data, state);      break;
        case BLUETOOTH_INFO_PAGE:/* BT info page has no data updates */ break;
        case MQTT_INFO_PAGE:     uiMqttInfoPageUpdate(data, state);      break;
        case BRIGHTNESS_PAGE:    uiBrightnessPageUpdate(data, state);    break;
        case VOLUME_PAGE:        uiVolumePageUpdate(data, state);        break;
        case LANGUAGE_PAGE:      uiLanguagePageUpdate(data, state);      break;
        default: break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// [API] GoTo: 压栈后跳转
// ─────────────────────────────────────────────────────────────────────────────
void uiManagerGoTo(int pageId) {
    if (pageId < 0 || pageId >= PAGE_POOL_SIZE) {
        Serial.printf("[UI-7] GoTo: invalid id=%d\n", pageId);
        return;
    }
    if (s_roots[pageId] == nullptr && !buildPage((UiPageId)pageId)) {
        Serial.printf("[UI-7] GoTo: page %d not built (unimplemented)\n", pageId);
        return;
    }
    if (pageId == (int)s_page) return;

    // 压栈（防溢出）
    if (s_navDepth < NAV_STACK_SIZE) {
        s_navStack[s_navDepth++] = (int)s_page;
    } else {
        Serial.println("[UI-7] nav stack full, overwriting oldest");
        // 向前移动减一位（丢弃最老的）
        memmove(s_navStack, s_navStack + 1, (NAV_STACK_SIZE - 1) * sizeof(int));
        s_navStack[NAV_STACK_SIZE - 1] = (int)s_page;
    }

    Serial.printf("[UI-7] GoTo %s (from %s, depth=%d)\n",
                  pageName((UiPageId)pageId), pageName(s_page), s_navDepth);
    s_page = (UiPageId)pageId;
    lv_scr_load(s_roots[pageId]);
}

// ─────────────────────────────────────────────────────────────────────────────
// [API] GoBack: 弹栈后返回
// ─────────────────────────────────────────────────────────────────────────────
void uiManagerGoBack() {
    if (s_navDepth == 0) {
        Serial.printf("[UI-7] GoBack: already at root (%s)\n", pageName(s_page));
        return;
    }
    int prev = s_navStack[--s_navDepth];
    Serial.printf("[UI-7] GoBack %s -> %s (depth=%d)\n",
                  pageName(s_page), pageName((UiPageId)prev), s_navDepth);
    s_page = (UiPageId)prev;
    lv_scr_load(s_roots[prev]);
}

// ─────────────────────────────────────────────────────────────────────────────
// [API] GetCurrentPage
// ─────────────────────────────────────────────────────────────────────────────
int uiManagerGetCurrentPage() {
    return (int)s_page;
}

// ─────────────────────────────────────────────────────────────────────────────
// [API] RefreshLang: 语言切换后刷新所有已创建页面文字
// ─────────────────────────────────────────────────────────────────────────────
void uiManagerRefreshLang() {
    Serial.printf("[UI-7] RefreshLang lang=%d\n", (int)uiI18nGetLanguage());

    // 更新所有页面的 Frame 标题 label
    struct { UiPageId id; UiStrKey key; } titleMap[] = {
        { HOME_PAGE,          STR_TITLE_HOME       },
        { MENU_PAGE,          STR_TITLE_MENU       },
        { SETTINGS_MENU_PAGE, STR_TITLE_SETTINGS   },
        { WIFI_INFO_PAGE,     STR_TITLE_WIFI       },
        { BLUETOOTH_INFO_PAGE,STR_TITLE_BLUETOOTH  },
        { MQTT_INFO_PAGE,     STR_TITLE_MQTT       },
        { BRIGHTNESS_PAGE,    STR_TITLE_BRIGHTNESS },
        { VOLUME_PAGE,        STR_TITLE_VOLUME     },
        { LANGUAGE_PAGE,      STR_TITLE_LANGUAGE   },
    };
    for (auto &tm : titleMap) {
        if (s_titleLbls[tm.id]) {
            lv_label_set_text(s_titleLbls[tm.id], uiI18nGet(tm.key));
        }
    }

    // 更新各页面内容区静态文字
    uiSettingsMenuPageRefreshLang();
    uiWifiInfoPageRefreshLang();
    uiBluetoothInfoPageRefreshLang();
    uiMqttInfoPageRefreshLang();
    uiBrightnessPageRefreshLang();
    uiVolumePageRefreshLang();
    uiLanguagePageRefreshLang();   // 同步高亮到已生效语言

    Serial.println("[UI-7] RefreshLang done");
}

// ─────────────────────────────────────────────────────────────────────────────
// [API] HandleKey: 全局按键分发
// ─────────────────────────────────────────────────────────────────────────────
int uiManagerHandleKey(uint32_t key) {
    if (key == UI_KEY_NONE) return (int)s_page;

    const char *kn = "?";
    if (key == UI_KEY_UP)    kn = "UP";
    if (key == UI_KEY_DOWN)  kn = "DOWN";
    if (key == UI_KEY_LEFT)  kn = "LEFT";
    if (key == UI_KEY_RIGHT) kn = "RIGHT";
    if (key == UI_KEY_OK)    kn = "OK";
    Serial.printf("[UI-7] key=%s page=%s\n", kn, pageName(s_page));

    // ── 全局拦截：LEFT = 返回上一页 ──────────────────────────────────────────
    if (key == UI_KEY_LEFT) {
        uiManagerGoBack();
        return (int)s_page;
    }

    // ── 按当前页面分发 ────────────────────────────────────────────────────────
    int result = (int)s_page;

    switch (s_page) {
        case HOME_PAGE:
            if (key == UI_KEY_RIGHT) uiManagerGoTo((int)MENU_PAGE);
            break;

        case MENU_PAGE: {
            result = uiMenuPageHandleKey(key);
            if (result != (int)MENU_PAGE) {
                uiManagerGoTo(result);
            }
            break;
        }

        case SETTINGS_MENU_PAGE: {
            result = uiSettingsMenuPageHandleKey(key);
            if (result != (int)SETTINGS_MENU_PAGE) {
                uiManagerGoTo(result);
            }
            break;
        }

        case BRIGHTNESS_PAGE:
            uiBrightnessPageHandleKey(key);   // 总返回 BRIGHTNESS_PAGE，LEFT 已被全局拦截
            break;

        case VOLUME_PAGE:
            uiVolumePageHandleKey(key);        // 总返回 VOLUME_PAGE，LEFT 已被全局拦截
            break;

        case LANGUAGE_PAGE:
            uiLanguagePageHandleKey(key);
            if (uiLanguagePageConsumeChanged()) {
                // 语言生效后立即全量刷新所有页面文字
                uiManagerRefreshLang();
            }
            break;

        // 纯展示页（Wi-Fi / BT / MQTT）：除 LEFT（已拦截）外忽略所有按键
        case WIFI_INFO_PAGE:
        case BLUETOOTH_INFO_PAGE:
        case MQTT_INFO_PAGE:
            break;

        default:
            break;
    }

    return (int)s_page;
}


// ─── end of ui_manager.cpp ───────────────────────────────────────────────────
// (old UI-5 implementation removed — superseded by UI-7 above)
