#include "ui_about_page.h"
#include "../../app_data.h"
#include "ui_manager.h" // 获取 ABOUT_PAGE 枚举
#include "ui_theme_pocketpc.h"

// ==========================================
// ℹ️ 关于本机页面实现 (About Page)
// ==========================================

// 内部静态指针，由于此页多为静态文本，其实不存指针也行，
// 但为了保持架构一致性和未来的可扩展性（如动态显示内存），依旧保留。
static lv_obj_t *deviceLabel  = nullptr;
static lv_obj_t *projectLabel = nullptr;
static lv_obj_t *versionLabel = nullptr;
static lv_obj_t *authorLabel  = nullptr;
static lv_obj_t *chipLabel    = nullptr;
static lv_obj_t *compileLabel = nullptr;

// ==========================================
// [接口] 兼容层：单独创建页面对象
// ==========================================
lv_obj_t* uiAboutPageCreate() {
    return lv_obj_create(NULL);
}

// ==========================================
// [接口] 初始化关于本机页面
// ==========================================
void uiAboutPageInit(lv_obj_t *parent) {
    // 采用弹性布局 (Flex) 垂直排列，模拟经典的系统信息面板
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    
    // 增加全局内边距
    lv_obj_set_style_pad_all(parent, 8, 0);
    lv_obj_set_style_pad_row(parent, 6, 0); // 行间距稍微紧凑一点，因为信息行数较多

    // 1. 设备名称
    deviceLabel = lv_label_create(parent);
    lv_label_set_text(deviceLabel, "Device: ESP Desktop");

    // 2. 项目名称
    projectLabel = lv_label_create(parent);
    lv_label_set_text(projectLabel, "Project: AIoT Pocket PC");

    // 3. 固件版本
    versionLabel = lv_label_create(parent);
    lv_label_set_text(versionLabel, "Firmware: v1.0.0");

    // 4. 作者 (开发者致谢)
    authorLabel = lv_label_create(parent);
    lv_label_set_text(authorLabel, "Author: zlight106");

    // 5. 芯片型号
    chipLabel = lv_label_create(parent);
    // 实际项目中可以使用 ESP.getChipModel() 获取，这里在 UI 层先写死占位
    lv_label_set_text(chipLabel, "Chip: ESP32 Series");

    // 6. 编译日期/时间
    compileLabel = lv_label_create(parent);
    // 使用 C/C++ 编译器内置宏，每次编译时自动写入最新时间
    lv_label_set_text_fmt(compileLabel, "Build: %s\n%s", __DATE__, __TIME__);
    lv_obj_set_style_text_color(compileLabel, lv_color_hex(0x666666), 0); // 编译时间用灰色显示
}

// ==========================================
// [接口] 页面数据刷新泵
// ==========================================
void uiAboutPageUpdate(const SensorData &data, const SystemState &state) {
    // 本页面大部分是静态写死的信息，通常不需要在主循环里高频刷新。
    // 如果后续你想在这里增加“动态可用堆内存 (Free Heap)”的显示，
    // 可以把 state.freeHeap 格式化后刷入一个新的 Label 中。
}

// ==========================================
// [接口] 页面销毁清理
// ==========================================
void uiAboutPageDeinit() {
    // 安全置空内部指针，根节点释放由 ui_manager 统一下达
    deviceLabel  = nullptr;
    projectLabel = nullptr;
    versionLabel = nullptr;
    authorLabel  = nullptr;
    chipLabel    = nullptr;
    compileLabel = nullptr;
}

// ==========================================
// [接口] 专属按键事件处理
// ==========================================
int uiAboutPageHandleKey(uint32_t key) {
    // [页面自身不触发旋律，由 ui_manager 控制]
    // 这里的 HandleKey 绝不能写 `speakerPlayAboutMelody()`！
    // 按照我们的解耦规则，音乐触发已经在 ui_manager.cpp 的 `GoTo` 和 `GoBack` 路由中实现了。
    
    // [页面自身不负责 click.wav]
    // 作为一个纯展示信息的叶子节点页面，这里没有任何可选中的菜单项。
    // 左键返回 (UI_KEY_LEFT) 已经被外层的 uiManager 拦截。
    // 遇到任何其他按键，都无视并返回当前的页面 ID。
    
    return ABOUT_PAGE;
}