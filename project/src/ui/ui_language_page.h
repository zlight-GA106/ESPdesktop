#pragma once
#include <lvgl.h>
#include "ui_i18n.h"  // UiLang 枚举

// 前向声明
struct SensorData;
struct SystemState;

// ==========================================
// 🌐 语言选择页面接口 (Language Page)
// 职责：显示两种语言选项，摇杆上下选择，OK 确认并立即生效
// ==========================================

// 初始化语言选择页面内容
// [参数]：parent - ui_manager 提供的内容区容器 (frame.content)
void uiLanguagePageInit(lv_obj_t *parent);

// 销毁清理（置空内部指针）
void uiLanguagePageDeinit();

// 数据刷新（保留契约接口，本页无需数据注入）
void uiLanguagePageUpdate(const SensorData &data, const SystemState &state);

// 按键事件处理
// UP/DOWN: 高亮切换选项
// OK:      应用所选语言（调用 uiI18nSetLanguage），标记 changed=true
// [返回值]: 始终返回 LANGUAGE_PAGE（不自行跳转）
int uiLanguagePageHandleKey(uint32_t key);

// 刷新自身文字（i18n 切换后由 ui_manager 调用）
void uiLanguagePageRefreshLang();

// 消费"语言已变更"标志（只返回 true 一次，之后复位）
// ui_manager 在 HandleKey 后调用此函数以决定是否触发全局 RefreshLang
bool uiLanguagePageConsumeChanged();
