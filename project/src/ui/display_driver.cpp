// ==========================================
// 显示驱动 - TFT_eSPI + LVGL (修复版)
// ==========================================
//
// 【修复记录】
// FIX-1: 添加 tft.setSwapBytes(true) —— LVGL 在 ESP32 上以小端存储 RGB565，
//        ST7735S 通过 SPI 接收时需要大端字节序。缺少 swap 导致蓝色变绿。
// FIX-2: flush 回调改用 startWrite/pushPixels/endWrite —— 确保 SPI 事务完整，
//        防止总线竞争导致的"脏像素/腐蚀"。
// FIX-3: 缓冲区从全屏双缓冲(80KB)缩减为 40 行单缓冲(10KB) ——
//        降低内存压力，避免潜在的栈/堆冲突。
// FIX-4: 移除手动硬件复位 —— tft.init() 内部已包含完整复位序列，
//        重复复位可能导致 ST7735S 进入未知状态。
// FIX-5: 项目 User_Setup.h 添加 USER_SETUP_LOADED —— 阻止 TFT_eSPI
//        加载库自带的空 User_Setup.h 造成配置冲突。

#include "display_driver.h"
#include "../../config.h"
#include "../fonts/ui_fonts.h"
#include "ui_style.h"
#include <esp_task_wdt.h>

// ★ 在 TFT_eSPI.h 之前 include 项目 User_Setup.h，其中已经定义了
//   USER_SETUP_LOADED、ST7735_DRIVER、引脚、SPI频率等所有配置。
#include "../../User_Setup.h"
#include <TFT_eSPI.h>
#include <lvgl.h>

// ==========================================
// 全局 TFT 实例
// ==========================================
TFT_eSPI tft = TFT_eSPI();

// ==========================================
// TFT 硬件基础初始化（TFT-only 和 LVGL 共用）
// ==========================================
static void tftHardwareInit() {
    Serial.println("[DISPLAY] TFT init...");
    tft.init();
    tft.setRotation(0);                 // 竖屏 128x160
    tft.setSwapBytes(true);             // ★ FIX-1: LVGL RGB565 小端 → ST7735S 大端
    tft.fillScreen(TFT_BLACK);

    // 背光
    if (TFT_BL != -1) {
        pinMode(TFT_BL, OUTPUT);
        digitalWrite(TFT_BL, HIGH);
    }
    Serial.println("[DISPLAY] TFT ready (swap=true, rot=0, 128x160)");
}

// ==========================================
// ---- 调试模式 1：纯 TFT 测试 ----
// ==========================================
void displayDriverInitTFTOnly() {
    tftHardwareInit();
    Serial.println("[DISPLAY] TFT-only mode (no LVGL)");
}

void displayDriverRunColorTest() {
    // 6 色循环测试
    struct ColorEntry { uint16_t color; const char* name; };
    ColorEntry tests[] = {
        { TFT_BLACK,   "BLACK"     },
        { TFT_WHITE,   "WHITE"     },
        { TFT_RED,     "RED"       },
        { TFT_GREEN,   "GREEN"     },
        { TFT_BLUE,    "BLUE"      },
        { TFT_NAVY,    "NAVY BLUE" },
    };

    for (int i = 0; i < 6; i++) {
        esp_task_wdt_reset();
        tft.fillScreen(tests[i].color);

        // 文字颜色：深色背景用白字，浅色背景用黑字
        uint16_t textColor = (tests[i].color == TFT_BLACK ||
                              tests[i].color == TFT_NAVY  ||
                              tests[i].color == TFT_BLUE   ||
                              tests[i].color == TFT_RED)
                             ? TFT_WHITE : TFT_BLACK;
        tft.setTextColor(textColor, tests[i].color);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(tests[i].name, 64, 80, 4);  // 居中, Font4

        Serial.printf("[COLOR-TEST] %d/6  %s  (0x%04X)\n",
                       i + 1, tests[i].name, tests[i].color);
        delay(2000);
    }
    Serial.println("[COLOR-TEST] done — all 6 colors shown");
}

// ==========================================
// ---- LVGL 完整模式：flush 回调 ----
// ==========================================

// ★ FIX-2: startWrite/pushPixels/endWrite 完整 SPI 事务
static void lv_flush_cb(lv_disp_drv_t *disp_drv,
                         const lv_area_t *area,
                         lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushPixels((uint16_t *)color_p, w * h);
    tft.endWrite();

    lv_disp_flush_ready(disp_drv);
}

// ★ FIX-6 (修订): LVGL 缓冲区必须在 DMA-capable 内部 SRAM 中
//   原因: tft.pushPixels() 使用 SPI DMA；DMA 引擎只能访问内部 SRAM，
//         PSRAM 地址对 DMA 不可见，会导致屏幕全黑。
//   使用 heap_caps_malloc(MALLOC_CAP_DMA|MALLOC_CAP_INTERNAL) 确保
//   分配的地址既在内部 SRAM 又满足 DMA 对齐要求。
#include <esp_heap_caps.h>
#define LV_BUF_LINES  40    // 40行 × 128px × 2bytes = 10240 bytes DMA SRAM
static lv_color_t *lv_buf = nullptr;

bool displayDriverBegin() {
    Serial.println("\n[DISPLAY] ===== displayDriverBegin =====");
    Serial.printf("[DISPLAY] heap free=%u  PSRAM free=%u\n",
                  esp_get_free_heap_size(),
                  heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    // 1. TFT 硬件
    tftHardwareInit();
    esp_task_wdt_reset();

    // 2. LVGL 核心
    Serial.println("[DISPLAY] lv_init...");
    lv_init();

    // 3. 分配 LVGL 缓冲区 —— 必须是 DMA-capable 内部 SRAM
    //    MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL = SPI DMA 可访问的内部 SRAM
    lv_buf = (lv_color_t*) heap_caps_malloc(
        128 * LV_BUF_LINES * sizeof(lv_color_t),
        MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (!lv_buf) {
        // 降级：普通内部 SRAM（仍然 DMA 可用，只是不保证对齐）
        lv_buf = (lv_color_t*) heap_caps_malloc(
            128 * LV_BUF_LINES * sizeof(lv_color_t), MALLOC_CAP_INTERNAL);
    }
    if (!lv_buf) {
        Serial.println("[DISPLAY] CRITICAL: lv_buf alloc failed! Halting.");
        while (true) { esp_task_wdt_reset(); delay(1000); }
    }
    Serial.printf("[DISPLAY] lv_buf=%p  %u bytes (DMA SRAM)\n",
                  (void*)lv_buf,
                  (unsigned)(128 * LV_BUF_LINES * sizeof(lv_color_t)));

    // 4. 注册显示驱动
    Serial.println("[DISPLAY] register disp driver (128x160)...");
    static lv_disp_draw_buf_t draw_buf;
    lv_disp_draw_buf_init(&draw_buf, lv_buf, NULL, 128 * LV_BUF_LINES);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res  = 128;
    disp_drv.ver_res  = 160;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.flush_cb = lv_flush_cb;
    lv_disp_drv_register(&disp_drv);

    // ── 统一样式初始化（字体加载日志 + 主题 + 屏幕字体）────────
    ui_style_init();

    esp_task_wdt_reset();
    Serial.printf("[DISPLAY] heap after LVGL init: free=%u\n", esp_get_free_heap_size());
    Serial.println("[DISPLAY] ===== displayDriverBegin OK =====\n");
    return true;
}

// ==========================================
// LVGL 心跳泵
// ==========================================
static unsigned long last_lv_tick = 0;
static unsigned long last_lv_handler = 0;

void displayDriverLoop() {
    unsigned long now = millis();

    // 首次进入 loop 时不要把整个 setup 耗时一次性灌给 LVGL。
    // 否则 lv_task_handler() 会在第一次调度里处理过大的 tick 增量，
    // 在 WiFi/AP 刚启动的窗口期放大 CPU 峰值。
    if (last_lv_tick == 0) {
        last_lv_tick = now;
    } else if (now != last_lv_tick) {
        lv_tick_inc(now - last_lv_tick);
        last_lv_tick = now;
    }

    // 5ms 节拍已经足够驱动 128x160 UI，避免每次 loop 都压满 CPU。
    if (now - last_lv_handler >= 5) {
        last_lv_handler = now;
        lv_task_handler();
    }
}
