#include "ui_ai_page.h"
#include "../../app_data.h"
#include "ui_manager.h" // 获取 AI_PAGE 枚举
#include "ui_keypad.h"
#include <stdio.h>

// ==========================================
// 🤖 AI 助手页面实现 (AI Assistant Page)
// ==========================================

// 内部静态控件指针，用于在 Update 中快速刷新文本
static lv_obj_t *statusLabel = nullptr;
static lv_obj_t *replyLabel  = nullptr;
static lv_obj_t *hintLabel   = nullptr;

// ==========================================
// [接口] 兼容层：单独创建页面对象
// ==========================================
lv_obj_t* uiAiPageCreate() {
    return lv_obj_create(NULL);
}

// ==========================================
// [接口] 初始化 AI 助手页面
// ==========================================
void uiAiPageInit(lv_obj_t *parent) {
    // 采用弹性布局 (Flex) 垂直排列
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    
    // 增加全局内边距
    lv_obj_set_style_pad_all(parent, 8, 0);
    lv_obj_set_style_pad_row(parent, 8, 0); // 行间距稍微紧凑一点，给长文本留空间

    // 1. AI 交互状态 (待机 / 请求中 / 播报中)
    statusLabel = lv_label_create(parent);
    lv_label_set_text(statusLabel, "Status: Standby");
    // 给状态文字加一点颜色区分 (例如深蓝色)，提升科技感
    lv_obj_set_style_text_color(statusLabel, lv_color_hex(0x000080), 0);

    // 2. 最后一条回复摘要 (多行文本)
    replyLabel = lv_label_create(parent);
    lv_label_set_text(replyLabel, "AI: Waiting for input...");
    // 开启长文本自动换行机制
    lv_label_set_long_mode(replyLabel, LV_LABEL_LONG_WRAP);
    // 【关键】必须限制文本框宽度，否则自动换行不会生效，会冲出屏幕边界！
    lv_obj_set_width(replyLabel, 112); 

    // 3. 操作提示
    hintLabel = lv_label_create(parent);
    lv_label_set_text(hintLabel, "[OK] to Talk/Stop");
    // 提示文字居中，并放置在屏幕最底部附近 (借助 Flex 特性自动往下排)
    lv_obj_set_style_text_color(hintLabel, lv_color_hex(0x666666), 0); // 浅灰色提示
}

// ==========================================
// [接口] 页面数据刷新泵
// ==========================================
void uiAiPageUpdate(const SensorData &data, const SystemState &state) {
    // [页面自身不直接发起 AI 请求]
    // UI 只做忠实的数据搬运工，根据数据白板里的状态刷新屏幕

    if (statusLabel) {
        // 假设 app_data.h 的 SystemState 中定义了 aiState 字段 (0=待机, 1=请求中, 2=播报中)
        // 此处为了兼容未定义的情况，先做伪代码逻辑。实际接入时替换为真实的 state.aiState 判断
        /*
        switch (state.aiState) {
            case 0: lv_label_set_text(statusLabel, "Status: Standby"); break;
            case 1: lv_label_set_text(statusLabel, "Status: Thinking..."); break;
            case 2: lv_label_set_text(statusLabel, "Status: Speaking"); break;
            default:lv_label_set_text(statusLabel, "Status: Unknown"); break;
        }
        */
    }

    if (replyLabel) {
        // 假设 SystemState 中有 char aiLastResponse[128]
        // lv_label_set_text_fmt(replyLabel, "AI: %s", state.aiLastResponse);
    }
}

// ==========================================
// [接口] 页面销毁清理
// ==========================================
void uiAiPageDeinit() {
    // [页面自身不负责退出停播]
    // 这里绝对不调用 audioPlayerStop()。
    // ui_manager.cpp 在 GoBack / GoTo 路由时，已经替我们干净利落地切断了流媒体！
    statusLabel = nullptr;
    replyLabel  = nullptr;
    hintLabel   = nullptr;
}

// ==========================================
// [接口] 专属按键事件处理
// ==========================================
int uiAiPageHandleKey(uint32_t key) {
    // [页面自身不负责 click.wav]
    // 只要不返回新的目标页面 ID，ui_manager 就不会越权播放 click.wav
    
    if (key == UI_KEY_OK) {
        // 用户按下了确认键，意图“开始录音”或“停止录音”
        
        // 【重要纪律】：UI 层绝对不允许直接调用 HTTP Client 去连接大模型 API！
        // 正确的做法是：在此处修改全局状态机标志位，或者向系统的消息队列抛出一个 Event。
        // Main Loop 中的 AI 业务线程检测到标志位后，再去驱动 I2S 麦克风录音并向云端发包。
        // 例如： systemState.aiRecordRequested = true;
        
        Serial.println("[UI] AI 语音交互被触发，已通知底层业务引擎处理");
    }

    // 维持在当前 AI 页面，左键返回的逻辑已经被全局拦截
    return AI_PAGE;
}