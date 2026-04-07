#include "ui_mqtt_info_page.h"
#include "../../app_data.h"
#include "ui_i18n.h"
#include "ui_theme_pocketpc.h"

// ==========================================
// ☁️ MQTT 云端信息页面实现 (MQTT Info Page)
// ==========================================

// 内部静态指针，用于在 Update 函数中快速定位并修改文本
static lv_obj_t *statusLabel = nullptr;
static lv_obj_t *brokerLabel = nullptr;
static lv_obj_t *haHintLabel = nullptr;

// ==========================================
// [接口] 兼容层：单独创建页面对象 (如需独立测试)
// ==========================================
lv_obj_t* uiMqttInfoPageCreate() {
    return lv_obj_create(NULL);
}

// ==========================================
// [接口] 初始化 MQTT 信息页面
// ==========================================
void uiMqttInfoPageInit(lv_obj_t *parent) {
    // 采用弹性布局 (Flex)，让信息条目垂直整齐排列
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    
    // 增加全局内边距，使文字不会紧贴着屏幕边框
    lv_obj_set_style_pad_all(parent, 8, 0);
    lv_obj_set_style_pad_row(parent, 12, 0); // 拉开行间距，保持商务感排版

    // 1. MQTT 连接状态
    statusLabel = lv_label_create(parent);
    lv_label_set_text_fmt(statusLabel, "%s%s",
        uiI18nGet(STR_MQTT_STATUS_LABEL), uiI18nGet(STR_MQTT_DISCONNECTED));

    // 2. Broker 服务器地址
    brokerLabel = lv_label_create(parent);
    lv_label_set_text_fmt(brokerLabel, "%s0.0.0.0:1883",
        uiI18nGet(STR_MQTT_BROKER_LABEL));

    // 3. Home Assistant 提示信息
    haHintLabel = lv_label_create(parent);
    lv_label_set_text(haHintLabel, uiI18nGet(STR_MQTT_HA_HINT));
    lv_label_set_long_mode(haHintLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(haHintLabel, 112);
}

// ==========================================
// [接口] 页面数据刷新泵
// ==========================================
void uiMqttInfoPageUpdate(const SensorData &data, const SystemState &state) {
    // [update 中可根据 state 刷新连接状态]
    
    if (statusLabel) {
        lv_label_set_text_fmt(statusLabel, "%s%s",
            uiI18nGet(STR_MQTT_STATUS_LABEL),
            state.mqttConnected ? uiI18nGet(STR_MQTT_CONNECTED)
                                 : uiI18nGet(STR_MQTT_DISCONNECTED));
    }

    if (brokerLabel) {
        lv_label_set_text_fmt(brokerLabel, "%s%s",
            uiI18nGet(STR_MQTT_BROKER_LABEL), state.mqttBroker);
    }
}

// ==========================================
// [接口] 页面销毁清理
// ==========================================
void uiMqttInfoPageDeinit() {
    // frame.root 会被 ui_manager 统一下达指令销毁。
    // 为了防止下次进入页面前触发 Update 导致野指针异常，必须安全置空。
    statusLabel = nullptr;
    brokerLabel = nullptr;
    haHintLabel = nullptr;
}

void uiMqttInfoPageRefreshLang() {
    if (statusLabel) lv_label_set_text_fmt(statusLabel, "%s%s",
        uiI18nGet(STR_MQTT_STATUS_LABEL), uiI18nGet(STR_MQTT_DISCONNECTED));
    if (brokerLabel) lv_label_set_text_fmt(brokerLabel, "%s0.0.0.0:1883",
        uiI18nGet(STR_MQTT_BROKER_LABEL));
    if (haHintLabel) lv_label_set_text(haHintLabel, uiI18nGet(STR_MQTT_HA_HINT));
}

// ==========================================
// [接口] 专属按键事件处理
// ==========================================
int uiMqttInfoPageHandleKey(uint32_t key) {
    // [页面自身不负责 click.wav]
    // 本页面是纯展示的叶子节点。根据系统架构，唯一的有效操作“左键返回 (UI_KEY_LEFT)”
    // 已经被外层的 uiManagerHandleKey 统一拦截，且规定返回时【不播放】click.wav。
    // 因此这里遇到任何其他按键（上下右确认），一律无视，直接返回当前页 ID 维持原状。
    
    return MQTT_INFO_PAGE;
}