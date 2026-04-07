/**
 * @file lv_conf.h
 * LVGL 配置文件 - 启用中文字体与显示优化
 * 支持中文字符、简体汉字标签、状态栏文本等显示需求
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>
#include <stdbool.h>

// ==========================================
// 【1】显示与图形基础配置
// ==========================================

/** 水平分辨率 */
#define LV_HOR_RES_MAX 128

/** 竖直分辨率 */
#define LV_VER_RES_MAX 160

/** 色彩深度 (16-bit RGB565) */
#define LV_COLOR_DEPTH 16

/** 内存优化 - 使用轻量级对象 */
#define LV_USE_OS LV_OS_NONE

// ==========================================
// 【2】字体配置 - 中文支持核心
// ==========================================

/**
 * LVGL 8.3+ 标准字体启用
 * 重要：不同版本的 LVGL 预编译的字体大小不同
 * 最保险的做法是启用最常用的几个大小
 */
#define LV_FONT_MONTSERRAT_8 1
#define LV_FONT_MONTSERRAT_10 1
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 0
#define LV_FONT_MONTSERRAT_16 0
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_20 0
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 0
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0

/** 
 * 启用中英文字体支持
 * UNSCII 可以作为备选字体
 */
#define LV_USE_FONT_UNSCII_8 0
#define LV_USE_FONT_UNSCII_16 0

/**
 * 设置默认字体 - 用于未显式指定字体的标签
 * 必须是已启用的字体大小
 * 注意：项目实际使用 ui_cn_font16_full，此处为 LVGL 内部 fallback
 */
#define LV_FONT_DEFAULT &lv_font_montserrat_12

/**
 * 启用内存优化字体子集
 * 只加载项目使用的字符，节省 Flash 空间
 */
#define LV_FONT_FMT_TXT_LARGE 0

// ==========================================
// 【3】文本与编码配置
// ==========================================

/**
 * UTF-8 编码支持 - 中文文本必须
 * 确保所有中文字符串正确解析
 */
#define LV_TXT_UTF8 1

/**
 * 绘文本中文换行宽度计算
 */
#define LV_TXT_BREAK_CHARS " .,!?;:,。，！？；："

// ==========================================
// 【4】对象与控件配置
// ==========================================

/** 启用标签 (Label) - 用于显示文本 */
#define LV_USE_LABEL 1

/** 启用按钮 (Button) - 用于界面框架按钮 */
#define LV_USE_BTN 1

/** 启用容器 (Container) - 用于页面布局 */
#define LV_USE_CONT 1

/** 启用按钮组 (Button Matrix) - 用于菜单项 */
#define LV_USE_BTNMATRIX 1

/** 启用滚动条支持 */
#define LV_USE_SCROLLBAR 1

// ==========================================
// 【5】动画与过渡配置
// ==========================================

/** 启用动画系统 - 用于 UI 反馈 */
#define LV_USE_ANIMATION 1

/** 最大同时动画数量 */
#define LV_ANIM_MAX_NUM 20

// ==========================================
// 【6】输入与交互配置
// ==========================================

/** 启用按键 (Keypad) 输入处理 */
#define LV_USE_INDEV_NULL 1

// ==========================================
// 【7】日志与调试配置
// ==========================================

/** 启用 LVGL 日志输出 */
#define LV_USE_LOG 1

/** 日志级别 (0=Trace, 1=Info, 2=Warning, 3=Error, 4=User) */
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN

/** 日志打印回调 (可与 Serial 集成) */
#define LV_LOG_PRINTF 0

// ==========================================
// 【8】性能与内存配置
// ==========================================

/**
 * 使用系统堆而不是 LVGL 固定 64KB 静态内存池
 * 原因：UI-7 多页 + WiFi 同时启用时，固定小内存池在第 3 页附近就可能耗尽。
 * 系统堆可自动利用 ESP32-S3 可用 RAM/PSRAM，避免预建/按需建页时卡死。
 */
#define LV_MEM_CUSTOM 1
#define LV_MEM_CUSTOM_INCLUDE <stdlib.h>
#define LV_MEM_CUSTOM_ALLOC   malloc
#define LV_MEM_CUSTOM_FREE    free
#define LV_MEM_CUSTOM_REALLOC realloc

/**
 * 部分刷新模式 - 仅更新变化区域
 * 减少 SPI 传输量，改善响应速度
 */
#define LV_DISP_ROT_MODES LV_DISP_ROT_NONE

/**
 * LV_MEM_CUSTOM=1 时此值被忽略，保留仅为注释参考。
 * 若后续回退到 LVGL 内置池，再恢复为实际容量配置。
 */
#define LV_MEM_SIZE (64 * 1024)

/** 对象内存池 */
#define LV_OBJ_CREATE_DEF_ATTR_SIZE 24

// ==========================================
// 【9】高级功能 (禁用以节省资源)
// ==========================================

/** 禁用 GPU 加速 (ST7735 无 GPU) */
#define LV_USE_GPU 0

/** 禁用 GPU 填充优化 */
#define LV_USE_GPU_STM32_DMA 0

/** 禁用 GPU 混合 */
#define LV_USE_GPU_ARM2D_GCC 0

/** 禁用风格过渡 */
#define LV_USE_STYLE_TRANSITION 0

/** 禁用阴影效果 (节省 CPU) */
#define LV_USE_SHADOW 0

// ==========================================
// 【10】测试与示例 (禁用在生产环境)
// ==========================================

#define LV_USE_EXAMPLES 0
#define LV_USE_DEMOS 0

// ==========================================
// 【自定义中文字体映射】
// ==========================================

/**
 * 由于 ESP32 Flash 空间有限，使用字体子集策略：
 * 1. 使用 Montserrat 字体作为基础
 * 2. 通过自定义字体库扩展中文支持
 * 3. 在 UI 初始化时映射常用中文字符
 * 
 * 常用中文字符将被映射到位图字体数据中
 * 查看 ui_theme_pocketpc.cpp 中的自定义字体初始化
 */

#endif /* LV_CONF_H */
