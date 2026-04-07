#include "ui_i18n.h"

// ── 当前激活语言 ──────────────────────────────────────────────────────────────
static UiLang s_lang = LANG_ZH_CN;

// ── 字符串表 [语言][键] ────────────────────────────────────────────────────────
// 行索引对应 UiLang 枚举值；列索引对应 UiStrKey 枚举值（必须与枚举顺序完全一致）。
// 中文行使用 UTF-8 编码；CJK 字符由 ui_cn_font16_full (16px) 渲染。
// clang-format off
static const char* s_strings[2][STR_KEY_COUNT] = {

// ─────────────────── LANG_ZH_CN ───────────────────────────────────────────
{
    // ── 页面标题 ────────────────────────────────────────────────────────────
    /* STR_TITLE_HOME         */ "主页",
    /* STR_TITLE_MENU         */ "菜单",
    /* STR_TITLE_SETTINGS     */ "设置",
    /* STR_TITLE_WIFI         */ "WiFi",
    /* STR_TITLE_BLUETOOTH    */ "蓝牙",
    /* STR_TITLE_MQTT         */ "MQTT",
    /* STR_TITLE_BRIGHTNESS   */ "屏幕亮度",
    /* STR_TITLE_VOLUME       */ "音量设置",
    /* STR_TITLE_LANGUAGE     */ "语言",
    /* STR_TITLE_SENSOR       */ "传感器",
    /* STR_TITLE_ABOUT        */ "关于本机",
    /* STR_TITLE_RADIO        */ "网络电台",
    /* STR_TITLE_AI           */ "AI助手",
    /* STR_TITLE_DINO         */ "恐龙游戏",

    // ── 主菜单项 ─────────────────────────────────────────────────────────────
    /* STR_MENU_SENSORS       */ "1.传感器",
    /* STR_MENU_SETTINGS      */ "2.设置",
    /* STR_MENU_RADIO         */ "3.网络电台",
    /* STR_MENU_AI            */ "4.AI助手",
    /* STR_MENU_DINO          */ "5.恐龙游戏",
    /* STR_MENU_ABOUT         */ "6.关于本机",

    // ── 设置菜单项 ───────────────────────────────────────────────────────────
    /* STR_SET_WIFI           */ "1.WiFi信息",
    /* STR_SET_BT             */ "2.蓝牙信息",
    /* STR_SET_MQTT           */ "3.MQTT信息",
    /* STR_SET_BRIGHTNESS     */ "4.屏幕亮度",
    /* STR_SET_VOLUME         */ "5.音量设置",
    /* STR_SET_LANGUAGE       */ "6.语言",

    // ── 状态栏前缀 (两种语言相同，WiFi/AI 是专有名词) ───────────────────────
    /* STR_STATUS_WIFI_PREFIX */ "WiFi:",
    /* STR_STATUS_AI_PREFIX   */ " AI:",

    // ── 状态栏值词 (随语言变化，需 CJK 字体渲染中文) ───────────────────────
    /* STR_STATUS_WIFI_OK     */ "连",      // 已连接 (单字，节省状态栏空间)
    /* STR_STATUS_WIFI_DASH   */ "--",      // 未连接 (ASCII，无需 CJK)
    /* STR_STATUS_AI_IDLE     */ "闲",      // 空闲
    /* STR_STATUS_AI_BUSY     */ "忙",      // 忙碌

    // ── Wi-Fi 信息页 ─────────────────────────────────────────────────────────
    /* STR_WIFI_STATUS_LABEL  */ "状态: ",
    /* STR_WIFI_IP_LABEL      */ "IP: ",
    /* STR_WIFI_MASK_LABEL    */ "掩码: ",
    /* STR_WIFI_MAC_LABEL     */ "MAC: ",
    /* STR_WIFI_CONNECTED     */ "已连接",
    /* STR_WIFI_DISCONNECTED  */ "未连接",

    // ── 蓝牙信息页 ───────────────────────────────────────────────────────────
    /* STR_BT_STATUS_LABEL    */ "状态: ",
    /* STR_BT_MAC_LABEL       */ "MAC: ",
    /* STR_BT_NOT_IMPL        */ "未启用",

    // ── MQTT 信息页 ──────────────────────────────────────────────────────────
    /* STR_MQTT_STATUS_LABEL  */ "状态: ",
    /* STR_MQTT_BROKER_LABEL  */ "Broker: ",
    /* STR_MQTT_HA_HINT       */ "HA发现: 开",
    /* STR_MQTT_CONNECTED     */ "已连接",
    /* STR_MQTT_DISCONNECTED  */ "未连接",

    // ── 亮度/音量格式字符串 ──────────────────────────────────────────────────
    /* STR_BRIGHTNESS_FMT     */ "亮度: %d%%",
    /* STR_VOLUME_FMT         */ "音量: %d%%",

    // ── 操作提示词 (与 LV_SYMBOL_LEFT 拼接显示 "←返") ──────────────────────
    /* STR_HINT_BACK          */ "返",

    // ── 语言选择项 ───────────────────────────────────────────────────────────
    /* STR_LANG_ZH_LABEL      */ "中文 (ZH)",
    /* STR_LANG_EN_LABEL      */ "English (EN)",
},

// ─────────────────── LANG_EN_US ───────────────────────────────────────────
{
    // ── 页面标题 ────────────────────────────────────────────────────────────
    /* STR_TITLE_HOME         */ "Home",
    /* STR_TITLE_MENU         */ "Menu",
    /* STR_TITLE_SETTINGS     */ "Settings",
    /* STR_TITLE_WIFI         */ "Wi-Fi",
    /* STR_TITLE_BLUETOOTH    */ "Bluetooth",
    /* STR_TITLE_MQTT         */ "MQTT",
    /* STR_TITLE_BRIGHTNESS   */ "Brightness",
    /* STR_TITLE_VOLUME       */ "Volume",
    /* STR_TITLE_LANGUAGE     */ "Language",
    /* STR_TITLE_SENSOR       */ "Sensors",
    /* STR_TITLE_ABOUT        */ "About",
    /* STR_TITLE_RADIO        */ "Web Radio",
    /* STR_TITLE_AI           */ "AI Asst.",
    /* STR_TITLE_DINO         */ "Dino Game",

    // ── 主菜单项 ─────────────────────────────────────────────────────────────
    /* STR_MENU_SENSORS       */ "1.Sensors",
    /* STR_MENU_SETTINGS      */ "2.Settings",
    /* STR_MENU_RADIO         */ "3.Web Radio",
    /* STR_MENU_AI            */ "4.AI Asst.",
    /* STR_MENU_DINO          */ "5.Dino Game",
    /* STR_MENU_ABOUT         */ "6.About",

    // ── 设置菜单项 ───────────────────────────────────────────────────────────
    /* STR_SET_WIFI           */ "1.Wi-Fi Info",
    /* STR_SET_BT             */ "2.Bluetooth",
    /* STR_SET_MQTT           */ "3.MQTT Info",
    /* STR_SET_BRIGHTNESS     */ "4.Brightness",
    /* STR_SET_VOLUME         */ "5.Volume",
    /* STR_SET_LANGUAGE       */ "6.Language",

    // ── 状态栏前缀 (两种语言相同) ────────────────────────────────────────────
    /* STR_STATUS_WIFI_PREFIX */ "WiFi:",
    /* STR_STATUS_AI_PREFIX   */ " AI:",

    // ── 状态栏值词 ───────────────────────────────────────────────────────────
    /* STR_STATUS_WIFI_OK     */ "OK",
    /* STR_STATUS_WIFI_DASH   */ "--",
    /* STR_STATUS_AI_IDLE     */ "Idle",
    /* STR_STATUS_AI_BUSY     */ "Busy",

    // ── Wi-Fi 信息页 ─────────────────────────────────────────────────────────
    /* STR_WIFI_STATUS_LABEL  */ "Status: ",
    /* STR_WIFI_IP_LABEL      */ "IP: ",
    /* STR_WIFI_MASK_LABEL    */ "Mask: ",
    /* STR_WIFI_MAC_LABEL     */ "MAC: ",
    /* STR_WIFI_CONNECTED     */ "Connected",
    /* STR_WIFI_DISCONNECTED  */ "Disconnected",

    // ── 蓝牙信息页 ───────────────────────────────────────────────────────────
    /* STR_BT_STATUS_LABEL    */ "Status: ",
    /* STR_BT_MAC_LABEL       */ "MAC: ",
    /* STR_BT_NOT_IMPL        */ "Not Enabled",

    // ── MQTT 信息页 ──────────────────────────────────────────────────────────
    /* STR_MQTT_STATUS_LABEL  */ "Status: ",
    /* STR_MQTT_BROKER_LABEL  */ "Broker: ",
    /* STR_MQTT_HA_HINT       */ "HA Discovery: On",
    /* STR_MQTT_CONNECTED     */ "Connected",
    /* STR_MQTT_DISCONNECTED  */ "Disconnected",

    // ── 亮度/音量格式字符串 ──────────────────────────────────────────────────
    /* STR_BRIGHTNESS_FMT     */ "Brightness: %d%%",
    /* STR_VOLUME_FMT         */ "Volume: %d%%",

    // ── 操作提示词 ───────────────────────────────────────────────────────────
    /* STR_HINT_BACK          */ "Bck",

    // ── 语言选择项 ───────────────────────────────────────────────────────────
    /* STR_LANG_ZH_LABEL      */ "ZH Chinese",
    /* STR_LANG_EN_LABEL      */ "EN English",
},

}; // end s_strings
// clang-format on

// ── API 实现 ────────────────────────────────────────────────────────────────

void uiI18nInit(UiLang lang) {
    s_lang = lang;
    Serial.printf("[i18n] init lang=%d\n", (int)lang);
}

void uiI18nSetLanguage(UiLang lang) {
    if (lang != LANG_ZH_CN && lang != LANG_EN_US) return;
    s_lang = lang;
    Serial.printf("[i18n] language -> %s\n", lang == LANG_ZH_CN ? "ZH" : "EN");
}

UiLang uiI18nGetLanguage() {
    return s_lang;
}

const char* uiI18nGet(UiStrKey key) {
    if (key < 0 || key >= STR_KEY_COUNT) return "";
    int lang = (int)s_lang;
    if (lang < 0 || lang > 1) lang = 0;
    const char *s = s_strings[lang][key];
    return s ? s : "";
}
