#include <Arduino.h>
#include "ui_frame.h"
#include "ui_theme_pocketpc.h"

// ==========================================
// 🪟 Pocket PC 2000 风格页面外框实现
// ==========================================

UiFrame uiFrameCreate(const char *title) {
    UiFrame frame;
    
    // -------------------------------------------------------
    // 1. 创建根节点 (Root Screen)
    // -------------------------------------------------------
    frame.root = lv_obj_create(NULL);
    lv_obj_add_style(frame.root, &style_page_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(frame.root, LV_OBJ_FLAG_SCROLLABLE);
    
    // -------------------------------------------------------
    // 2. 创建顶部标题栏容器 (Top Bar)
    // [微调] 高度从 22→26px，让按钮和标题更从容，不再显得挤；
    //        下方状态栏/内容区起点随之下移。
    // -------------------------------------------------------
    lv_obj_t *top_bar = lv_obj_create(frame.root);
    lv_obj_set_size(top_bar, 128, 24);
    lv_obj_align(top_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_style(top_bar, &style_title_bar, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(top_bar, LV_OBJ_FLAG_SCROLLABLE);
    
    // -------------------------------------------------------
    // 3. 创建左侧返回按钮 (Back Button)
    // -------------------------------------------------------
    frame.backBtn = lv_btn_create(top_bar);
    lv_obj_set_size(frame.backBtn, 30, 18);
    lv_obj_align(frame.backBtn, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_style(frame.backBtn, &style_button_normal, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(frame.backBtn, &style_button_pressed, LV_PART_MAIN | LV_STATE_PRESSED);
    
    lv_obj_t *back_label = lv_label_create(frame.backBtn);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT);  // 左箭头图标，避免中文字体问题
    lv_obj_center(back_label);
    
    // -------------------------------------------------------
    // 4. 创建右侧菜单按钮 (Menu Button)
    // [微调] 高度从 18→20px，与返回按钮保持统一
    // -------------------------------------------------------
    frame.menuBtn = lv_btn_create(top_bar);
    lv_obj_set_size(frame.menuBtn, 30, 18);
    lv_obj_align(frame.menuBtn, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_style(frame.menuBtn, &style_button_normal, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(frame.menuBtn, &style_button_pressed, LV_PART_MAIN | LV_STATE_PRESSED);
    
    lv_obj_t *menu_label = lv_label_create(frame.menuBtn);
    lv_label_set_text(menu_label, LV_SYMBOL_LIST);  // 列表图标，避免中文字体问题
    lv_obj_center(menu_label);
    
    // -------------------------------------------------------
    // 5. 创建中间标题文本 (Title Label)
    // [FIX] 固定宽60px，避免撑破到按钮区
    // 可用中区 = 128 - pad_l(2) - pad_r(2) - backBtn(30) - menuBtn(30) = 64px
    // 取保守值60px；LV_LABEL_LONG_DOT 超长标题显示"..."，不溢出
    // -------------------------------------------------------
    frame.titleLabel = lv_label_create(top_bar);
    lv_label_set_text(frame.titleLabel, title);
    lv_label_set_long_mode(frame.titleLabel, LV_LABEL_LONG_DOT);
    lv_obj_set_width(frame.titleLabel, 60);
    lv_obj_add_style(frame.titleLabel, &style_title_text, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(frame.titleLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(frame.titleLabel, LV_ALIGN_CENTER, 0, 0);
    
    // -------------------------------------------------------
    // 6. 创建状态栏 (Status Bar)
    // [布局]：独立一行，紧贴标题栏下方，宽度128px，高度14px
    // [视觉]：中灰底、黑字，与标题栏和内容区形成三层视觉分区
    // [原因]：128px宽屏幕标题栏放不下按钮+标题+状态，必须独立一行
    // -------------------------------------------------------
    // [FIX] 初始文本仅含 WiFi+AI；高度16px给font_14留2px呼吸空间
    // y=26 紧接加高后的顶栏（26px）下方，不重叠
    frame.statusLabel = lv_label_create(frame.root);
    lv_label_set_text(frame.statusLabel, "WIFI: --  AI: IDLE");
    lv_label_set_long_mode(frame.statusLabel, LV_LABEL_LONG_CLIP);  // 超宽时裁剪单行，不折行
    lv_obj_add_style(frame.statusLabel, &style_status_text, LV_PART_MAIN | LV_STATE_DEFAULT);
    // font_zh_12 line_height=17，留1px上下边距 → statusBar 高度19px
    lv_obj_set_size(frame.statusLabel, 128, 19);
    lv_obj_align(frame.statusLabel, LV_ALIGN_TOP_MID, 0, 24);  // 紧接顶栏(24px)下方
    
    // -------------------------------------------------------
    // 7. 创建主内容区容器 (Content Container)
    // [布局] 顶栏(26px) + 状态栏(16px) = 42px
    //        内容区高度 = 160 - 42 = 118px
    // -------------------------------------------------------
    frame.content = lv_obj_create(frame.root);
    // 顶栏 24px + 状态栏 19px (font_zh_12 适配) = 43px
    lv_obj_set_size(frame.content, 128, 160 - 24 - 19);  // 117px
    lv_obj_align(frame.content, LV_ALIGN_BOTTOM_MID, 0, 0);
    
    lv_obj_add_style(frame.content, &style_content_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(frame.content, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(frame.content, 0, LV_PART_MAIN);
    
    // 开启 Y 轴滚动（如果子控件超出138px的高度）
    lv_obj_set_scroll_dir(frame.content, LV_DIR_VER);
    // 隐藏碍眼的滚动条
    lv_obj_set_scrollbar_mode(frame.content, LV_SCROLLBAR_MODE_OFF);
    
    Serial.println("[FRAME] ✓ 页面外框创建完成");
    return frame;
}
