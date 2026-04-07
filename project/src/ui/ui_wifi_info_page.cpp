#include "ui_wifi_info_page.h"
#include "../../app_data.h"
#include "ui_i18n.h"
#include "ui_theme_pocketpc.h"

// ==========================================
// 📶 Wi-Fi 信息页面实现 (Wi-Fi Info Page)
// ==========================================

// 内部静态指针，用于在 Update 函数中快速定位并修改文本
static lv_obj_t *statusLabel = nullptr;
static lv_obj_t *ipLabel = nullptr;
static lv_obj_t *maskLabel = nullptr;
static lv_obj_t *macLabel = nullptr;

// ==========================================
// [接口] 兼容层：单独创建页面对象 (如需独立测试)
// ==========================================
lv_obj_t* uiWifiInfoPageCreate() {
    return lv_obj_create(NULL);
}

// ==========================================
// [接口] 初始化 Wi-Fi 信息页面
// ==========================================
void uiWifiInfoPageInit(lv_obj_t *parent) {
    // 采用弹性布局 (Flex)，让信息条目从上到下整齐排列
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    
    // 增加全局内边距，使文字不会紧贴着屏幕边框
    lv_obj_set_style_pad_all(parent, 8, 0);
    lv_obj_set_style_pad_row(parent, 12, 0); // 行间距拉开一点，避免信息拥挤

    // 1. Wi-Fi 连接状态
    statusLabel = lv_label_create(parent);
    lv_label_set_text_fmt(statusLabel, "%s--", uiI18nGet(STR_WIFI_STATUS_LABEL));

    // 2. IP 地址
    ipLabel = lv_label_create(parent);
    lv_label_set_text_fmt(ipLabel, "%s0.0.0.0", uiI18nGet(STR_WIFI_IP_LABEL));

    // 3. 子网掘码
    maskLabel = lv_label_create(parent);
    lv_label_set_text_fmt(maskLabel, "%s0.0.0.0", uiI18nGet(STR_WIFI_MASK_LABEL));

    // 4. MAC 地址
    macLabel = lv_label_create(parent);
    lv_label_set_text_fmt(macLabel, "%s00:00:00:00:00:00", uiI18nGet(STR_WIFI_MAC_LABEL));
}

// ==========================================
// [接口] 页面数据刷新泵
// ==========================================
void uiWifiInfoPageUpdate(const SensorData &data, const SystemState &state) {
    // [当前网络信息可先占位，后续可接真实值]
    // 假设 app_data.h 的 SystemState 结构体中已经定义了这些网络字段。
    // 如果还没定义，代码当前会直接展示 Init 里的占位符文本。
    
    if (statusLabel) {
        lv_label_set_text_fmt(statusLabel, "%s%s",
            uiI18nGet(STR_WIFI_STATUS_LABEL),
            state.wifiConnected ? uiI18nGet(STR_WIFI_CONNECTED)
                                 : uiI18nGet(STR_WIFI_DISCONNECTED));
    }

    if (ipLabel) {
        lv_label_set_text_fmt(ipLabel, "%s%s", uiI18nGet(STR_WIFI_IP_LABEL), state.wifiIp);
    }

    if (maskLabel) {
        lv_label_set_text_fmt(maskLabel, "%s%s", uiI18nGet(STR_WIFI_MASK_LABEL), state.wifiMask);
    }

    if (macLabel) {
        lv_label_set_text_fmt(macLabel, "%s%s", uiI18nGet(STR_WIFI_MAC_LABEL), state.wifiMac);
    }
}

// ==========================================
// [接口] 页面销毁清理
// ==========================================
void uiWifiInfoPageDeinit() {
    // frame.root 已经被 ui_manager 统一下达指令销毁，
    // 为了防止下一次进入页面前触发 Update 导致野指针异常，必须安全置空。
    statusLabel = nullptr;
    ipLabel = nullptr;
    maskLabel = nullptr;
    macLabel = nullptr;
}

void uiWifiInfoPageRefreshLang() {
    if (statusLabel) lv_label_set_text_fmt(statusLabel, "%s--", uiI18nGet(STR_WIFI_STATUS_LABEL));
    if (ipLabel)    lv_label_set_text_fmt(ipLabel,    "%s0.0.0.0", uiI18nGet(STR_WIFI_IP_LABEL));
    if (maskLabel)  lv_label_set_text_fmt(maskLabel,  "%s0.0.0.0", uiI18nGet(STR_WIFI_MASK_LABEL));
    if (macLabel)   lv_label_set_text_fmt(macLabel,   "%s00:00:00:00:00:00", uiI18nGet(STR_WIFI_MAC_LABEL));
}

// ==========================================
// [接口] 专属按键事件处理
// ==========================================
int uiWifiInfoPageHandleKey(uint32_t key) {
    // [页面自身不负责 click.wav]
    // 作为一个末端信息展示页，用户在这里唯一能做的按键交互就是“返回上一页”。
    // 根据系统架构，左键返回 (UI_KEY_LEFT) 已经被外层的 uiManagerHandleKey 统一拦截，
    // 并且我们规定了返回时【不播放】click.wav。
    // 因此这里遇到任何其他按键（上下右确认），都无视并返回当前页 ID。
    
    return WIFI_INFO_PAGE;
}