#include "ui_bluetooth_info_page.h"
#include "../../app_data.h"
#include "ui_i18n.h"
#include "ui_theme_pocketpc.h"

// ==========================================
// 蓝牙信息页面实现 (Bluetooth Info Page)
// ==========================================

// 内部静态指针，用于 Update 函数快速定位并修改文本
static lv_obj_t *statusLabel = nullptr;
static lv_obj_t *macLabel = nullptr;

// ==========================================
// [接口] 兼容层：单独创建页面对象 (如需独立测试)
// ==========================================
lv_obj_t* uiBluetoothInfoPageCreate() {
    return lv_obj_create(NULL);
}

// ==========================================
// [接口] 初始化蓝牙信息页面
// ==========================================
void uiBluetoothInfoPageInit(lv_obj_t *parent) {
    // 采用弹性布局 (Flex)，让信息条目垂直整齐排列
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    
    // 增加一点全局内边距，使文字不会紧贴着屏幕边框
    lv_obj_set_style_pad_all(parent, 8, 0);
    lv_obj_set_style_pad_row(parent, 12, 0); // 行间距

    // 1. 蓝牙连接或启用状态
    statusLabel = lv_label_create(parent);
    lv_label_set_text_fmt(statusLabel, "%s%s",
        uiI18nGet(STR_BT_STATUS_LABEL), uiI18nGet(STR_BT_NOT_IMPL));

    // 2. 蓝牙硬件 MAC 地址
    macLabel = lv_label_create(parent);
    lv_label_set_text_fmt(macLabel, "%s00:00:00:00:00:00",
        uiI18nGet(STR_BT_MAC_LABEL));
}

// ==========================================
// [接口] 页面数据刷新泵
// ==========================================
void uiBluetoothInfoPageUpdate(const SensorData &data, const SystemState &state) {
    // [允许占位]
    // 假设未来 app_data.h 的 SystemState 结构体中加入了 bleEnabled / bleMac 字段。
    // 在真实底层逻辑写好之前，这里可以暂时留空或者只做防御性更新。
    
    /* 预留的真实更新代码：
    if (statusLabel) {
        if (state.bleEnabled) {
            lv_label_set_text(statusLabel, "Status: Enabled / Ready");
        } else {
            lv_label_set_text(statusLabel, "Status: Disabled");
        }
    }

    if (macLabel) {
        lv_label_set_text_fmt(macLabel, "MAC: %s", state.bleMac);
    }
    */
}

// ==========================================
// [接口] 页面销毁清理
// ==========================================
void uiBluetoothInfoPageDeinit() {
    // ui_manager 会统一销毁 frame.root，为了防止下次进入该页面前
    // 触发 Update 导致野指针异常，必须将悬空指针安全置空。
    statusLabel = nullptr;
    macLabel = nullptr;
}

void uiBluetoothInfoPageRefreshLang() {
    if (statusLabel) lv_label_set_text_fmt(statusLabel, "%s%s",
        uiI18nGet(STR_BT_STATUS_LABEL), uiI18nGet(STR_BT_NOT_IMPL));
    if (macLabel) lv_label_set_text_fmt(macLabel, "%s00:00:00:00:00:00",
        uiI18nGet(STR_BT_MAC_LABEL));
}

// ==========================================
// [接口] 专属按键事件处理
// ==========================================
int uiBluetoothInfoPageHandleKey(uint32_t key) {
    // [页面自身不负责 click.wav]
    // 作为一个末端的纯展示页面，用户在这里的唯一有效操作是按下“左键”返回。
    // 根据我们严密的系统架构，左键 (UI_KEY_LEFT) 已经被外层的 uiManagerHandleKey 
    // 全局拦截，并且规定了返回上一页时不播放 click.wav 音效。
    // 因此这里遇到任何其他按键（上下右确认），都无视并返回当前页 ID，维持静默状态。
    
    return BLUETOOTH_INFO_PAGE;
}