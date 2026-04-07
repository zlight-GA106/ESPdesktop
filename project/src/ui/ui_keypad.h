#pragma once
#include <Arduino.h>

// ==========================================
// 🕹️ 五向摇杆输入层驱动接口 (Keypad)
// 职责：将物理摇杆的 GPIO 电平状态抽象为统一的按键指令
// ==========================================

// 定义统一按键常量 (抽象层语义，与具体物理引脚解耦)
#define UI_KEY_NONE   0  // 无按键按下 (释放状态)
#define UI_KEY_UP     1  // 向上 (常用于菜单上移)
#define UI_KEY_DOWN   2  // 向下 (常用于菜单下移)
#define UI_KEY_LEFT   3  // 向左 (常用于返回上一级、减小音量)
#define UI_KEY_RIGHT  4  // 向右 (常用于进入下一级、增大音量)
#define UI_KEY_OK     5  // 确认 (按下摇杆，常用于确认或打开下级页面)

// 初始化五向摇杆硬件
// [作用]：配置 config.h 中定义的 5 个摇杆引脚，将它们设置为内部上拉输入 (INPUT_PULLUP)。
// [返回值]：初始化成功返回 true。
bool uiKeypadBegin();

// 扫描并读取当前摇杆的物理按压状态
// [作用]：检测哪一个方向的引脚被拉低（接 GND），并将对应的抽象键值赋给传入的引用变量。
// [参数]：key - 引用传递，用于接收当前的按键常量 (如 UI_KEY_UP)
// [返回值]：如果有按键被按下，返回 true；如果处于空闲松开状态，返回 false 且 key 被置为 UI_KEY_NONE。
// [注意]：此函数将被 LVGL 的底层输入轮询定时器高频调用 (indev_read_cb)。
bool uiKeypadRead(uint32_t &key);