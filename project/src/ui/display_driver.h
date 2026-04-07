#pragma once
#include <Arduino.h>

// ==========================================
// 显示与图形底层驱动接口 (TFT_eSPI + LVGL)
// 职责：桥接物理屏幕硬件与 LVGL 渲染引擎
// ==========================================

// ---- TFT-only 接口 (调试模式 1) ----

// 仅初始化 TFT 硬件，不加载 LVGL
void displayDriverInitTFTOnly();

// 循环播放 6 色测试画面（黑/白/红/绿/蓝/海军蓝），每色停留 2 秒
void displayDriverRunColorTest();

// ---- LVGL 完整接口 (调试模式 2/3) ----

// 初始化 TFT 硬件 + LVGL 栈（flush 回调、缓冲区、显示驱动注册）
bool displayDriverBegin();

// LVGL 渲染心跳泵，必须在 loop() 中高频调用
void displayDriverLoop();