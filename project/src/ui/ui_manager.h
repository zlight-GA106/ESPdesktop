#pragma once
#include <Arduino.h>
#include <lvgl.h>
#include "../../app_data.h"  // ⚡ 核心：引用全局唯一的数据字典与页面 ID 枚举

// ==========================================
// 🗺️ UI 路由与页面生命周期管理器
// 职责：统一管理所有页面的创建、销毁、按键分发与数据刷新
// ==========================================

// ==========================================
// [接口] UI 管理器核心方法
// ==========================================

/**
 * @brief 初始化 UI 管理器
 * [职责]：在物理屏幕和 LVGL 初始化完毕后调用。
 * 它会初始化内部的页面历史栈，并主动加载第一个页面 (通常是 HOME_PAGE)。
 */
void uiManagerBegin();

/**
 * @brief 全局 UI 数据刷新泵
 * [职责]：必须在主程序的 loop() 中定时调用。
 * 管理器会将最新的全局数据 (SensorData 和 SystemState) 
 * 注入并刷新给当前处于活跃状态的页面，同时更新顶部的统一状态栏。
 */
void uiManagerUpdate(const SensorData &data, const SystemState &state);

/**
 * @brief 全局按键事件分发器
 * [职责]：接收底层 ui_keypad 读取到的物理按键指令。
 * 管理器会拦截系统级按键 (如全局返回)，或者将按键透传给当前活跃页面去处理。
 * [返回值]：返回当前活跃的页面 ID (便于扩展或调试)。
 */
int uiManagerHandleKey(uint32_t key);

/**
 * @brief 核心路由：前进/跳转到新页面
 * [职责]：
 * 1. 将当前页面 ID 压入历史记录栈。
 * 2. 调用当前页面的销毁函数，防止内存泄漏。
 * 3. 根据 pageId 加载并渲染目标页面。
 */
void uiManagerGoTo(int pageId);

/**
 * @brief 核心路由：后退/返回上一页
 * [职责]：响应物理左键或 UI 返回按钮。
 * 从历史栈弹出旧页面 ID，销毁当前页，重装上一页，并执行音频停播策略。
 */
void uiManagerGoBack();

/**
 * @brief 获取当前页面 ID（调试心跳用）
 */
int uiManagerGetCurrentPage();

/**
 * @brief 语言切换后刷新所有已创建页面的文字
 * [职责]：刷新所有已创建页面的标题栏 label 和内容区静态文字，
 * 使语言切换立即生效。由 uiManagerHandleKey 在 LANGUAGE_PAGE OK 确认后自动调用。
 */
void uiManagerRefreshLang();