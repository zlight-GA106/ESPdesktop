#pragma once
#include <Arduino.h>
// ui_i18n.h — 双语言字符串查找表接口
// 支持语言: LANG_ZH_CN (中文) / LANG_EN_US (English)
// 注意: 中文 UTF-8 字符串需要 CJK 字体才能在屏幕上正确显示。
//       当前统一字体: ui_cn_font16_full (16px, BPP=2)，通过 ui_font_default() 获取。

// ── 语言枚举 (定义在此，app_data.h 通过前向声明使用) ──────────────────────────
enum UiLang {
    LANG_ZH_CN = 0,   // 简体中文 (默认)
    LANG_EN_US = 1    // English
};

// ── 字符串键枚举 ──────────────────────────────────────────────────────────────
enum UiStrKey {
    // 页面标题 (全部页面)
    STR_TITLE_HOME = 0,
    STR_TITLE_MENU,
    STR_TITLE_SETTINGS,
    STR_TITLE_WIFI,
    STR_TITLE_BLUETOOTH,
    STR_TITLE_MQTT,
    STR_TITLE_BRIGHTNESS,
    STR_TITLE_VOLUME,
    STR_TITLE_LANGUAGE,
    STR_TITLE_SENSOR,     // 传感器详情页标题
    STR_TITLE_ABOUT,      // 关于本机页标题
    STR_TITLE_RADIO,      // 网络电台页标题
    STR_TITLE_AI,         // AI 助手页标题
    STR_TITLE_DINO,       // 小恐龙游戏页标题

    // 主菜单项文字
    STR_MENU_SENSORS,
    STR_MENU_SETTINGS,
    STR_MENU_RADIO,
    STR_MENU_AI,
    STR_MENU_DINO,
    STR_MENU_ABOUT,

    // 设置菜单项文字
    STR_SET_WIFI,
    STR_SET_BT,
    STR_SET_MQTT,
    STR_SET_BRIGHTNESS,
    STR_SET_VOLUME,
    STR_SET_LANGUAGE,

    // 状态栏：前缀标签 (WiFi:/AI: 在两种语言中相同)
    STR_STATUS_WIFI_PREFIX,   // "WiFi:"
    STR_STATUS_AI_PREFIX,     // " AI:"

    // 状态栏：状态值词 (随语言变化)
    STR_STATUS_WIFI_OK,       // 已连 / OK
    STR_STATUS_WIFI_DASH,     // -- (无连接，两种语言相同)
    STR_STATUS_AI_IDLE,       // 闲 / Idle
    STR_STATUS_AI_BUSY,       // 忙 / Busy

    // Wi-Fi 信息页
    STR_WIFI_STATUS_LABEL,
    STR_WIFI_IP_LABEL,
    STR_WIFI_MASK_LABEL,
    STR_WIFI_MAC_LABEL,
    STR_WIFI_CONNECTED,
    STR_WIFI_DISCONNECTED,

    // 蓝牙信息页
    STR_BT_STATUS_LABEL,
    STR_BT_MAC_LABEL,
    STR_BT_NOT_IMPL,

    // MQTT 信息页
    STR_MQTT_STATUS_LABEL,
    STR_MQTT_BROKER_LABEL,
    STR_MQTT_HA_HINT,
    STR_MQTT_CONNECTED,
    STR_MQTT_DISCONNECTED,

    // 亮度/音量页格式字符串 (%d 用于插入数值)
    STR_BRIGHTNESS_FMT,
    STR_VOLUME_FMT,

    // 亮度/音量页操作提示 (上↑/下↓/左←)
    STR_HINT_BACK,    // "返" / "Bck"  — 与 LV_SYMBOL_LEFT 拼接使用

    // 语言选择页
    STR_LANG_ZH_LABEL,
    STR_LANG_EN_LABEL,

    // ── 数组边界哨兵 ──────────────────────────────
    STR_KEY_COUNT
};

// ── API ──────────────────────────────────────────────────────────────────────

/**
 * @brief 初始化 i18n 模块，设置默认语言
 * @param lang  初始语言，默认为 LANG_ZH_CN
 */
void uiI18nInit(UiLang lang = LANG_ZH_CN);

/**
 * @brief 运行时切换语言（仅修改内存，不写 Flash）
 * @param lang  目标语言
 */
void uiI18nSetLanguage(UiLang lang);

/**
 * @brief 获取当前激活的语言枚举
 */
UiLang uiI18nGetLanguage();

/**
 * @brief 根据键查找当前语言的字符串
 * @param key  UiStrKey 枚举值
 * @return const char*  永不返回 nullptr（越界时返回空串 ""）
 */
const char* uiI18nGet(UiStrKey key);
