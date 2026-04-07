#pragma once
// ==========================================================
// ui_text.h — 统一字符串管理（双语言扩展接口）
//
// 基础宏：提供常用 UI 文本的 UTF-8 字面量常量
// 语言接口：通过 ui_text_get(lang, id) 获取指定语言的文本
//           内部转发到已有的 ui_i18n 系统，保持一致性
// ==========================================================

#include "ui_i18n.h"   // 引入 UiLang, UiStrKey, uiI18nGet()

// ──────────────────────────────────────────────────────────
// 常用文本宏（直接使用时的 UTF-8 字面量，不依赖运行时）
// ──────────────────────────────────────────────────────────
#define TXT_HOME         "\xe4\xb8\xbb\xe9\xa1\xb5"       // 主页
#define TXT_MENU         "\xe8\x8f\x9c\xe5\x8d\x95"       // 菜单
#define TXT_SETTINGS     "\xe8\xae\xbe\xe7\xbd\xae"       // 设置
#define TXT_WIFI         "WiFi"
#define TXT_BLUETOOTH    "\xe8\x93\x9d\xe7\x89\x99"       // 蓝牙
#define TXT_MQTT         "MQTT"
#define TXT_TIME         "\xe6\x97\xb6\xe9\x97\xb4"       // 时间
#define TXT_SYSTEM       "\xe7\xb3\xbb\xe7\xbb\x9f"       // 系统
#define TXT_SENSOR       "\xe4\xbc\xa0\xe6\x84\x9f\xe5\x99\xa8" // 传感器
#define TXT_RADIO        "\xe7\xbd\x91\xe7\xbb\x9c\xe7\x94\xb5\xe5\x8f\xb0" // 网络电台
#define TXT_AI           "AI\xe5\x8a\xa9\xe6\x89\x8b"    // AI助手
#define TXT_ABOUT        "\xe5\x85\xb3\xe4\xba\x8e"       // 关于
#define TXT_BACK         "\xe8\xbf\x94\xe5\x9b\x9e"       // 返回
#define TXT_OK           "\xe7\xa1\xae\xe8\xae\xa4"       // 确认
#define TXT_CANCEL       "\xe5\x8f\x96\xe6\xb6\x88"       // 取消
#define TXT_BRIGHTNESS   "\xe4\xba\xae\xe5\xba\xa6"       // 亮度
#define TXT_VOLUME       "\xe9\x9f\xb3\xe9\x87\x8f"       // 音量
#define TXT_LANGUAGE     "\xe8\xaf\xad\xe8\xa8\x80"       // 语言

// ──────────────────────────────────────────────────────────
// 语言 ID 枚举（与 ui_i18n.h 中 UiLang 完全对齐）
// 这里重新声明是为了让用户通过 ui_text.h 单文件使用
// ──────────────────────────────────────────────────────────
// UiLang 枚举已在 ui_i18n.h 定义：
//   LANG_ZH_CN = 0  // 简体中文
//   LANG_EN_US = 1  // English

// ──────────────────────────────────────────────────────────
// ui_text_get() — 双语言文本获取接口
//
// @param lang  目标语言 (LANG_ZH_CN / LANG_EN_US)
// @param id    字符串键 (UiStrKey 枚举，定义于 ui_i18n.h)
// @return      对应语言的 UTF-8 字符串字面量
//
// 示例：
//   lv_label_set_text(label, ui_text_get(LANG_ZH_CN, STR_TITLE_SETTINGS));
//   lv_label_set_text(label, ui_text_get(LANG_EN_US, STR_TITLE_SETTINGS));
// ──────────────────────────────────────────────────────────
static inline const char* ui_text_get(UiLang lang, UiStrKey id)
{
    // 临时切换语言，查询后恢复原语言
    UiLang cur = uiI18nGetLanguage();
    if (cur != lang) uiI18nSetLanguage(lang);
    const char* s = uiI18nGet(id);
    if (cur != lang) uiI18nSetLanguage(cur);
    return s;
}
