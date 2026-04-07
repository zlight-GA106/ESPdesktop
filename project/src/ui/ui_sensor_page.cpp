#include "ui_sensor_page.h"
#include "../../app_data.h"
#include "ui_manager.h" // 获取 SENSOR_PAGE 枚举

// ==========================================
// 📊 传感器详情页面实现 (Sensor Page)
// ==========================================

// 内部静态指针，用于 Update 函数快速定位并修改内容
static lv_obj_t *tempHumiLabel = nullptr;
static lv_obj_t *lightLabel = nullptr;
static lv_obj_t *radarLabel = nullptr;

// RGB 色块相关指针
static lv_obj_t *rgbContainer = nullptr;
static lv_obj_t *rgbLabel = nullptr;
static lv_obj_t *rgbColorBox = nullptr;

// ==========================================
// [接口] 兼容层：单独创建页面对象 (如需独立测试)
// ==========================================
lv_obj_t* uiSensorPageCreate() {
    return lv_obj_create(NULL);
}

// ==========================================
// [接口] 初始化传感器页面
// ==========================================
void uiSensorPageInit(lv_obj_t *parent) {
    // 使用 Flex 垂直布局，让各个传感器信息像列表一样整齐排列
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    // 左对齐，顶部对齐
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    // 增加一点内边距，让文字不要死死贴着屏幕边缘
    lv_obj_set_style_pad_all(parent, 8, 0);
    lv_obj_set_style_pad_row(parent, 10, 0); // 设置行与行之间的间距

    // 1. 温湿度数据区
    tempHumiLabel = lv_label_create(parent);
    lv_label_set_text(tempHumiLabel, "Temp: --.- C\nHumi: --.- %");

    // 2. 光照数据区
    lightLabel = lv_label_create(parent);
    lv_label_set_text(lightLabel, "Light: -- Lux");

    // 3. 人体存在雷达状态区
    radarLabel = lv_label_create(parent);
    lv_label_set_text(radarLabel, "Radar: Unknown");

    // 4. RGB 联动状态区
    // (使用一个水平布局的透明容器，把“文字”和“色块”横向拼在一起)
    rgbContainer = lv_obj_create(parent);
    lv_obj_remove_style_all(rgbContainer); // 移除默认的背景和边框，变成纯透明容器
    lv_obj_set_size(rgbContainer, 120, 24);
    lv_obj_set_flex_flow(rgbContainer, LV_FLEX_FLOW_ROW); // 横向排列
    lv_obj_set_flex_align(rgbContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(rgbContainer, 8, 0);      // 文字和色块之间的间距

    rgbLabel = lv_label_create(rgbContainer);
    lv_label_set_text(rgbLabel, "RGB Link:");

    // 创建展示真实颜色的色块 (Color Box)
    rgbColorBox = lv_obj_create(rgbContainer);
    lv_obj_set_size(rgbColorBox, 20, 16);
    lv_obj_set_style_radius(rgbColorBox, 0, 0); // 绝对直角，剥夺现代圆角
    lv_obj_set_style_border_width(rgbColorBox, 1, 0);
    lv_obj_set_style_border_color(rgbColorBox, lv_color_black(), 0); // 黑色硬边框
    lv_obj_set_style_bg_color(rgbColorBox, lv_color_hex(0x000000), 0); // 默认黑色
}

// ==========================================
// [接口] 页面数据刷新泵
// ==========================================
void uiSensorPageUpdate(const SensorData &data, const SystemState &state) {
    // 1. 刷新温湿度
    if (tempHumiLabel) {
        lv_label_set_text_fmt(tempHumiLabel, "Temp: %.1f C\nHumi: %.1f %%", data.temperature, data.humidity);
    }

    // 2. 刷新光照
    if (lightLabel) {
        lv_label_set_text_fmt(lightLabel, "Light: %.0f Lux", data.lightLevel);
    }

    // 3. 刷新雷达人体存在状态
    if (radarLabel) {
        if (data.humanDetected) {
            lv_label_set_text(radarLabel, "Radar: Detected!");
        } else {
            lv_label_set_text(radarLabel, "Radar: Clear");
        }
    }

    // 4. 刷新 RGB 色块颜色
    // [只负责显示，不直接控制真实 RGB]
    // 这里的 UI 是绝对的“旁观者”，它只从数据白板 (state) 里读取颜色并刷新界面色块，
    // 真正的 WS2812B 硬件发光，是由专门的硬件任务层去解析 state 并在后台点亮的。
    if (rgbColorBox) {
        // 假设 app_data.h 的 SystemState 中含有 rgbR, rgbG, rgbB 字段
        lv_color_t currentColor = lv_color_make(state.rgbR, state.rgbG, state.rgbB);
        lv_obj_set_style_bg_color(rgbColorBox, currentColor, 0);
    }
}

// ==========================================
// [接口] 页面销毁清理
// ==========================================
void uiSensorPageDeinit() {
    // 根节点会被 ui_manager 统一下达指令销毁，此处必须将悬空指针安全置空
    tempHumiLabel = nullptr;
    lightLabel = nullptr;
    radarLabel = nullptr;
    rgbContainer = nullptr;
    rgbLabel = nullptr;
    rgbColorBox = nullptr;
}

// ==========================================
// [接口] 专属按键事件处理
// ==========================================
int uiSensorPageHandleKey(uint32_t key) {
    // 这是一个纯粹的数据展示面板，内部没有任何需要用户“上下选择”的交互菜单。
    // 左键返回 (UI_KEY_LEFT) 已经被 ui_manager 全局拦截了。
    // 所以这里遇到任何按键，都直接返回当前的 SENSOR_PAGE，维持原状。
    return SENSOR_PAGE;
}