#pragma once
#include <lvgl.h>
#include <stdint.h>

// 前向声明数据结构，避免与系统数据总线产生互相包含的头文件灾难
struct SensorData;
struct SystemState;

// ==========================================
// 🤖 AI 助手页面接口 (AI Assistant Page)
// 职责：展示语音对话流与大模型交互状态
// ==========================================

// 创建 AI 助手页面内容对象 (满足你明确要求的声明)
// [返回值]：返回该页面的底层容器对象指针
lv_obj_t* uiAiPageCreate();

// 刷新 AI 页面全局数据 (满足你明确要求的声明)
// [作用]：由 ui_manager 在主循环中高频定时调用。负责从 state 中提取
// 最新的对话文本、录音状态或音频播放状态，并刷新到界面的多行文本框或动画控件上。
void uiAiPageUpdate(const SensorData &data, const SystemState &state);

// ==========================================
// ⚠️ UI 管理器生命周期与路由契约
// 为了配合 ui_manager.cpp 中已经写死的压栈路由机制，必须补充以下标准接口。
// ==========================================

// 初始化 AI 助手页面
// [参数]：parent - ui_manager 提供的内容区容器 (frame.content)
void uiAiPageInit(lv_obj_t *parent);

// 销毁 AI 助手页面
// [作用]：在页面退出时安全释放内部指针，停止所有相关的 UI 动画定时器。
// 注意：退出停播音频的逻辑已由外层 ui_manager 统管，此处仅清理内存。
void uiAiPageDeinit();

// 处理 AI 助手页面的按键事件
// [参数]：key - 摇杆按键枚举
// [返回值]：返回期望跳转的目标页面 ID。遇到按键触发语音录制或停止，返回键退出。
int uiAiPageHandleKey(uint32_t key);