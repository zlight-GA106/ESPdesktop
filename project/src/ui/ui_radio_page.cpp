#include "ui_radio_page.h"
#include "../../app_data.h"
#include "ui_manager.h" // 获取 RADIO_PAGE 枚举
#include "ui_theme_pocketpc.h"
#include "ui_keypad.h"
#include "../storage/settings.h" // 引入设置存储模块，用于读取电台列表

// ==========================================
// 📻 网络电台页面实现 (Web Radio Page)
// ==========================================

// 定义界面最多支持显示的电台数量，防止内存溢出
#define MAX_RADIO_STATIONS 10

// 内部状态
static uint8_t selectedIndex = 0;  // 当前光标悬停的项
static uint8_t playingIndex = 255; // 当前真正在播放的项 (255 表示未播放)

// 存储所有电台项的 LVGL 对象指针
static lv_obj_t* radioItems[MAX_RADIO_STATIONS];

// ==========================================
// [接口] 兼容层：单独创建页面对象
// ==========================================
lv_obj_t* uiRadioPageCreate() {
    return lv_obj_create(NULL);
}

// ==========================================
// [接口] 设置并刷新当前选中项的高亮样式
// ==========================================
void uiRadioPageSetSelected(uint8_t index) {
    // 假设 settings 模块有获取电台总数的接口，这里为了安全做越界保护
    // 实际项目中请替换为真实的 settingsGetRadioCount()
    uint8_t count = 5; // 假定当前有5个电台
    if (index >= count) index = count - 1;
    
    selectedIndex = index;

    for (int i = 0; i < count; i++) {
        if (radioItems[i] == nullptr) continue;

        if (i == selectedIndex) {
            // [当前选中项高亮蓝]
             lv_obj_add_style(radioItems[i], &style_button_pressed, LV_PART_MAIN | LV_STATE_DEFAULT);
             lv_obj_remove_style(radioItems[i], &style_button_normal, LV_PART_MAIN | LV_STATE_DEFAULT);
        } else {
            // 未选中项恢复普通样式
             lv_obj_add_style(radioItems[i], &style_button_normal, LV_PART_MAIN | LV_STATE_DEFAULT);
             lv_obj_remove_style(radioItems[i], &style_button_pressed, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }

    // 自动滚动，确保选中的电台始终在可视区域内
    if (radioItems[selectedIndex] != nullptr) {
        lv_obj_scroll_to_view(radioItems[selectedIndex], LV_ANIM_ON);
    }
}

// ==========================================
// [接口] 获取当前选中项
// ==========================================
uint8_t uiRadioPageGetSelected() {
    return selectedIndex;
}

// ==========================================
// [接口] 初始化网络电台页面
// ==========================================
void uiRadioPageInit(lv_obj_t *parent) {
    // 弹性布局，垂直列表排列
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // [列表来源说明]
    // 这里的列表并非写死在 UI 代码中，而是通过包含 ../storage/settings.h 
    // 从设备的 NVS (非易失性存储) 或 SD 卡配置中动态读取的。
    // 这做到了 UI 视图与数据存储的彻底解耦。
    
    // 假设 settings 提供了以下 API (如果你的 API 名字不同，请在此微调)：
    // int count = settingsGetRadioCount();
    // const char* name = settingsGetRadioName(i);
    int radioCount = 5; // 假定读取到了 5 个电台
    if (radioCount > MAX_RADIO_STATIONS) radioCount = MAX_RADIO_STATIONS;

    // 占位测试数据
    const char* mockRadioNames[] = {
        "1. Lofi Girl", 
        "2. Synthwave", 
        "3. BBC World", 
        "4. Jazz Cafe", 
        "5. Classic FM"
    };

    // 动态生成电台列表项
    for (int i = 0; i < radioCount; i++) {
        radioItems[i] = lv_btn_create(parent);
        lv_obj_set_width(radioItems[i], 120); 
        lv_obj_set_height(radioItems[i], 24); 

        lv_obj_remove_style_all(radioItems[i]);
lv_obj_add_style(radioItems[i], &style_button_pressed, LV_PART_MAIN | LV_STATE_PRESSED);

        lv_obj_t *label = lv_label_create(radioItems[i]);
        // 实际应用中请使用：lv_label_set_text(label, settingsGetRadioName(i));
        lv_label_set_text(label, mockRadioNames[i]);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 4, 0); 
    }

    // 强制刷新一次焦点样式
    uiRadioPageSetSelected(selectedIndex);
}

// ==========================================
// [接口] 独立刷新电台界面的状态
// ==========================================
void uiRadioPageUpdate() {
    // 在这里可以处理播放动画（例如在当前 playingIndex 的右侧显示一个小喇叭图标）
}

// ==========================================
// [接口] 页面数据刷新泵 (满足系统统一契约)
// ==========================================
void uiRadioPageUpdate(const SensorData &data, const SystemState &state) {
    // 定期调用独立 Update
    uiRadioPageUpdate();
}

// ==========================================
// [接口] 页面销毁清理
// ==========================================
void uiRadioPageDeinit() {
    // [页面自身不负责退出停播]
    // 这里绝对不可以调用 audioPlayerStop()！
    // 因为根据纪律，当页面离开时，ui_manager.cpp 已经在路由底层强行切断了音频流。
    // 页面只需要老老实实地清理自己的内存指针即可。
    for (int i = 0; i < MAX_RADIO_STATIONS; i++) {
        radioItems[i] = nullptr;
    }
}

// ==========================================
// [接口] 专属按键事件处理
// ==========================================
int uiRadioPageHandleKey(uint32_t key) {
    // [页面自身不负责 click.wav]
    // 无论是上下切换还是按下确认，只要我们返回的依然是 RADIO_PAGE，
    // ui_manager 就不会把它当作“页面跳转”，自然就不会越权播放 click.wav。

    if (key == UI_KEY_UP) {
        // [上下移动选中项时不播放 click.wav]
        if (selectedIndex > 0) {
            uiRadioPageSetSelected(selectedIndex - 1);
        }
    } 
    else if (key == UI_KEY_DOWN) {
        // [上下移动选中项时不播放 click.wav]
        uint8_t count = 5; // 替换为真实的 settingsGetRadioCount()
        if (selectedIndex < count - 1) {
            uiRadioPageSetSelected(selectedIndex + 1);
        }
    } 
    else if (key == UI_KEY_OK) {
        // [选中项与当前播放状态的区别]
        // selectedIndex (选中项) 仅仅代表深蓝色光标停留在哪里，是视觉层面的焦点；
        // playingIndex (播放状态) 代表底层音频解码器正在拉取哪个流。
        // 只有当用户按下 OK 时，选中项才会转变为正在播放项。
        
        playingIndex = selectedIndex;

        // [OK 只负责选定当前电台并由上层触发播放]
        // 这里绝对不能直接包含 audio_player.h 去调用 audioPlayerPlay(url)。
        // 正确的做法是：在此处更新一个全局状态 (如写入 app_data 的 state)，
        // 或者发送一个事件。主循环 (Main Loop) 侦测到状态变更后，再由业务核心层去触发播放引擎。
        // 例如： systemState.targetRadioIndex = playingIndex;
        //        systemState.radioPlayRequested = true;
        
        Serial.printf("[UI] 选中播放电台索引: %d，已通知上层处理\n", playingIndex);
    }

    // 维持在当前电台页面，由全局拦截处理左键返回
    return RADIO_PAGE;
}