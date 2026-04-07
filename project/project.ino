#include <Arduino.h>
#include <esp_task_wdt.h>
#include <esp_rom_sys.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// ==========================================
// ★★★ 显示调试模式开关 ★★★
// ==========================================
// 1 = 纯 TFT 色彩自检（完全绕开 LVGL，验证底层颜色/字节序）
// 2 = 最小 LVGL 测试（海军蓝全屏 + 白色文字，不加载任何 UI 框架）
// 3 = 完整 UI-1 框架（Theme + Frame，正式视觉骨架）
// 4 = 字体验证（ui_cn_font16_full 英文+中文混排，验证无撕裂）
// 5 = 正式 UI 系统（ui_main_init: 主界面/设置/信息 三页导航）
#define DISPLAY_DEBUG_MODE  3

#include <lvgl.h>
#include <WiFi.h>             // wifiManagerBeginProvisioning() 在 setup() 中调用
#include <esp_heap_caps.h>    // heap_caps_get_free_size / MALLOC_CAP_SPIRAM
#include "src/ui/display_driver.h"

#include "app_data.h"  // SensorData, SystemState definitions

#if DISPLAY_DEBUG_MODE == 3
#include "src/ui/ui_theme_pocketpc.h"
#include "src/ui/ui_keypad.h"
#include "src/ui/ui_manager.h"
#include "src/storage/settings.h"      // settingsBegin / settingsGet
#include "src/net/web_server.h"        // webServerShouldReboot
#include "src/net/wifi_manager.h"      // wifiManagerBeginProvisioning / wifiManagerLoop
#endif

#if DISPLAY_DEBUG_MODE >= 2
#include "src/ui/ui_style.h"
#endif

#if DISPLAY_DEBUG_MODE == 5
#include "src/ui/ui_main.h"
#endif

// ==========================================
// 打印复位原因
// ==========================================
void printResetReason() {
    esp_reset_reason_t reason = esp_reset_reason();
    Serial.print("[RESET] reason: ");
    switch(reason) {
        case ESP_RST_POWERON:   Serial.println("power-on"); break;
        case ESP_RST_SW:        Serial.println("software"); break;
        case ESP_RST_PANIC:     Serial.println("PANIC!"); break;
        case ESP_RST_INT_WDT:   Serial.println("INT_WDT!"); break;
        case ESP_RST_TASK_WDT:  Serial.println("TASK_WDT!"); break;
        case ESP_RST_WDT:       Serial.println("WDT!"); break;
        case ESP_RST_BROWNOUT:  Serial.println("BROWNOUT!"); break;
        default:                Serial.println("other"); break;
    }
}

// ==========================================
// 看门狗
// ==========================================
void initWatchdog() {
    esp_task_wdt_init(30, true);
    Serial.println("[WDT] 30s timeout");
}

// ==========================================
// 全局数据（extern 声明在 app_data.h，此处定义实体）
// ==========================================
SensorData  g_sensorData;
SystemState g_systemState;

#if DISPLAY_DEBUG_MODE == 3
static QueueHandle_t s_inputQueue = nullptr;
static TaskHandle_t  s_lvglTaskHandle = nullptr;
static TaskHandle_t  s_inputTaskHandle = nullptr;
static TaskHandle_t  s_networkTaskHandle = nullptr;
static volatile bool s_mode3TasksReady = false;
static bool s_networkBootstrapped = false;

static void drainInputQueue() {
    if (!s_inputQueue) return;

    uint32_t key = UI_KEY_NONE;
    while (xQueueReceive(s_inputQueue, &key, 0) == pdTRUE) {
        uiManagerHandleKey(key);
    }
}

void lvgl_task(void *arg)
{
    (void)arg;
    esp_task_wdt_add(NULL);

    while (!s_mode3TasksReady)
    {
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    Serial.printf("[LVGL] task run core=%d\n", xPortGetCoreID());

    unsigned long lastUpdate = millis();
    unsigned long lastAlive  = 0;

    while (1)
    {
        displayDriverLoop();
        drainInputQueue();

        unsigned long now = millis();
        if (now - lastUpdate >= 200) {
            lastUpdate = now;
            uiManagerUpdate(g_sensorData, g_systemState);
        }

        if (now - lastAlive >= 5000) {
            lastAlive = now;
            Serial.printf("[LVGL] alive page=%d t=%lums\n",
                          uiManagerGetCurrentPage(), now);
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void input_task(void *arg)
{
    (void)arg;
    esp_task_wdt_add(NULL);

    while (!s_mode3TasksReady)
    {
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    Serial.printf("[INPUT] task run core=%d\n", xPortGetCoreID());

    unsigned long lastAlive = 0;

    while (1)
    {
        uint32_t key = UI_KEY_NONE;
        if (uiKeypadRead(key) && s_inputQueue) {
            xQueueSend(s_inputQueue, &key, 0);
        }

        unsigned long now = millis();
        if (now - lastAlive >= 5000) {
            lastAlive = now;
            Serial.printf("[INPUT] alive t=%lums\n", now);
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

static void network_task(void *arg)
{
    (void)arg;
    esp_task_wdt_add(NULL);

    while (!s_mode3TasksReady)
    {
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    unsigned long lastAlive = 0;

    Serial.printf("[NET] task start core=%d heap=%u psram=%u\n",
                  xPortGetCoreID(),
                  esp_get_free_heap_size(),
                  heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    while (1)
    {
        if (!s_networkBootstrapped) {
            s_networkBootstrapped = true;
            Serial.println("[NET] WiFi bootstrap begin");
            wifiManagerBeginProvisioning();
            Serial.println("[NET] WiFi bootstrap end");
        }

        wifiManagerLoop();

        if (webServerShouldReboot()) {
            Serial.println("[NET] deferred reboot now");
            vTaskDelay(pdMS_TO_TICKS(100));
            ESP.restart();
        }

        unsigned long now = millis();
        if (now - lastAlive >= 5000) {
            lastAlive = now;
            Serial.printf("[NET] alive state=%d t=%lums\n",
                          (int)wifi_get_state(), now);
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static bool createMode3Tasks() {
    s_mode3TasksReady = false;

    Serial.println("[TASK] create: queue");
    s_inputQueue = xQueueCreate(8, sizeof(uint32_t));
    if (!s_inputQueue) {
        Serial.println("[TASK] input queue create failed");
        return false;
    }
    Serial.println("[TASK] queue ok");

    Serial.println("[TASK] create: lvgl");
    if (xTaskCreatePinnedToCore(lvgl_task, "lvgl", 8192, NULL, 2,
                                &s_lvglTaskHandle, 1) != pdPASS) {
        Serial.println("[TASK] lvgl task create failed");
        return false;
    }
    Serial.println("[TASK] lvgl core=1 prio=2");

    Serial.println("[TASK] create: input");
    if (xTaskCreatePinnedToCore(input_task, "input", 4096, NULL, 1,
                                &s_inputTaskHandle, 1) != pdPASS) {
        Serial.println("[TASK] input task create failed");
        return false;
    }
    Serial.println("[TASK] input core=1 prio=1");

    Serial.println("[TASK] create: network");
    if (xTaskCreatePinnedToCore(network_task, "network", 6144, NULL, 1,
                                &s_networkTaskHandle, 0) != pdPASS) {
        Serial.println("[TASK] network task create failed");
        return false;
    }
    Serial.println("[TASK] network core=0 prio=1");

    s_mode3TasksReady = true;
    Serial.println("[TASK] all tasks released");
    return true;
}
#endif



// ==========================================
// Setup
// ==========================================
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n\n========================================");
    Serial.printf("  ESPdesktop  DISPLAY_DEBUG_MODE = %d\n", DISPLAY_DEBUG_MODE);
    Serial.println("========================================");
    printResetReason();
    initWatchdog();
    esp_task_wdt_reset();

    // ==========================================
    // MODE 1: 纯 TFT 色彩自检
    // ==========================================
#if DISPLAY_DEBUG_MODE == 1
    Serial.println("\n[MODE-1] Pure TFT color test (no LVGL)");
    displayDriverInitTFTOnly();
    Serial.println("[MODE-1] Starting color loop...");
    while (true) {
        esp_task_wdt_reset();
        displayDriverRunColorTest();
        Serial.println("[MODE-1] cycle done, restarting...");
    }
#endif

    // ==========================================
    // MODE 2: 最小 LVGL 颜色验证
    // ==========================================
#if DISPLAY_DEBUG_MODE == 2
    Serial.println("\n[MODE-2] Minimal LVGL color test");
    displayDriverBegin();
    esp_task_wdt_reset();

    // 海军蓝全屏背景
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000080), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

    // 白色居中文本
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "LVGL COLOR TEST");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, ui_font_default(), LV_PART_MAIN);
    lv_obj_center(label);

    Serial.println("[MODE-2] theme init ok");
    Serial.println("[MODE-2] frame create ok");
#endif

    // ==========================================
    // MODE 3: UI-7 设置页面树 + 双语言导航
    // ==========================================
#if DISPLAY_DEBUG_MODE == 3
    Serial.println("\n[MODE-3] UI-7 settings tree + i18n");

    // ── 0. NVS（WiFi 凭据在 wifiManagerBeginProvisioning 里需要）──
    settingsBegin();
    Serial.println("[SETUP] settings ok");
    esp_task_wdt_reset();

    // ── 1. 显示驱动 + LVGL ────────────────────────────────────────
    displayDriverBegin();
    esp_task_wdt_reset();

    // ── 2. PocketPC 主题 + 按键 + UI 管理器 ──────────────────────
    uiThemePocketPcInit();
    Serial.println("[UI-5] theme ok");
    esp_task_wdt_reset();

    uiKeypadBegin();
    Serial.println("[UI-5] keypad ok");
    esp_task_wdt_reset();

    uiManagerBegin();
    Serial.println("[UI-5] manager ok");
    esp_task_wdt_reset();

    Serial.println("[SETUP] WiFi deferred to network task");

    // ── 3. 创建三大任务：LVGL / INPUT / NETWORK ────────────────
    if (!createMode3Tasks()) {
        Serial.println("[TASK] create failed, halt");
        while (true) {
            esp_task_wdt_reset();
            delay(1000);
        }
    }
#endif

    // ==========================================
    // MODE 4: 字体验证 — ui_cn_font16_full 英文/中文混排
    //   label0 (白居中) : "Hello 123 设置 WiFi 系统 OK" — 最终验证
    //   label1 (绿) : "Hello 123 设置 网络 蓝牙 系统 OK!" — 混排测试
    //   label2 (白) : "设置 WiFi 时间 系统" — CJK+ASCII
    //   label3 (黄) : "Hello ABC abc 0123" — 纯 ASCII
    //   成功标准：英文清晰、中文可辨认、无方块乱码。
    // ==========================================
#if DISPLAY_DEBUG_MODE == 4
    Serial.println("\n[MODE-4] Font verification: ui_cn_font16_full");
    displayDriverBegin();  // displayDriverBegin 内部调用 ui_style_init()
    esp_task_wdt_reset();

    lv_obj_t *scr4 = lv_scr_act();
    lv_obj_set_style_bg_color(scr4, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr4, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_text_font(scr4, ui_font_default(), 0);

    // ── 最终验证标签（居中白色）──
    lv_obj_t *label_verify = lv_label_create(scr4);
    lv_label_set_text(label_verify,
        "Hello 123 "
        "\xe8\xae\xbe\xe7\xbd\xae"   /* 设置 */
        " WiFi "
        "\xe7\xb3\xbb\xe7\xbb\x9f"   /* 系统 */
        " OK");
    lv_obj_set_style_text_color(label_verify, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_verify, ui_font_default(), LV_PART_MAIN);
    lv_obj_center(label_verify);

    // ── 混排全测标签（绿）──
    lv_obj_t *label_full = lv_label_create(scr4);
    lv_label_set_text(label_full,
        "Hello 123"
        " \xe8\xae\xbe\xe7\xbd\xae"   /* 设置 */
        " \xe7\xbd\x91\xe7\xbb\x9c"   /* 网络 */
        " \xe8\x93\x9d\xe7\x89\x99"   /* 蓝牙 */
        " \xe7\xb3\xbb\xe7\xbb\x9f"   /* 系统 */
        " OK!");
    lv_obj_set_style_text_color(label_full, lv_color_hex(0x00FF00), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_full, ui_font_default(), LV_PART_MAIN);
    lv_obj_align(label_full, LV_ALIGN_TOP_LEFT, 4, 20);

    // ── 中文标签（白）──
    lv_obj_t *label_cjk = lv_label_create(scr4);
    lv_label_set_text(label_cjk,
        "\xe8\xae\xbe\xe7\xbd\xae WiFi"
        " \xe6\x97\xb6\xe9\x97\xb4"
        " \xe7\xb3\xbb\xe7\xbb\x9f");
    lv_obj_set_style_text_color(label_cjk, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_cjk, ui_font_default(), LV_PART_MAIN);
    lv_obj_align(label_cjk, LV_ALIGN_TOP_LEFT, 4, 56);

    // ── 英文标签（黄）──
    lv_obj_t *label_latin = lv_label_create(scr4);
    lv_label_set_text(label_latin, "Hello ABC abc 0123");
    lv_obj_set_style_text_color(label_latin, lv_color_hex(0xFFFF00), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_latin, ui_font_default(), LV_PART_MAIN);
    lv_obj_align(label_latin, LV_ALIGN_TOP_LEFT, 4, 92);

    Serial.println("[MODE-4] labels created: Verify / Mixed / CJK / Latin");
    esp_task_wdt_reset();
#endif

    // ==========================================
    // MODE 5: 正式 UI 系统 — 主界面/设置/信息 三页导航
    // ==========================================
#if DISPLAY_DEBUG_MODE == 5
    Serial.println("\n[MODE-5] Full UI system: ui_main_init");
    displayDriverBegin();       // 内部已调用 ui_style_init()
    esp_task_wdt_reset();
    ui_main_init();             // 加载首页 PAGE_HOME
    Serial.println("[MODE-5] ready — HOME page loaded");
    esp_task_wdt_reset();
#endif

    esp_task_wdt_reset();
    Serial.println("\n[SETUP] done");
}

void loop() {
    // 所有实际工作都已转移到独立 FreeRTOS 任务；主 loop 只做空闲让步。
    vTaskDelay(pdMS_TO_TICKS(1000));
}
