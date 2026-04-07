#include "ui_dino_page.h"
#include "../../app_data.h"
#include "ui_manager.h" // 获取 DINO_PAGE 枚举
#include "ui_keypad.h"
#include <Arduino.h>    // 引入 millis() 用于控制游戏帧率

// ==========================================
// 🦖 恐龙小游戏彩蛋页面实现 (Dino Game Page)
// ==========================================

// --- 游戏物理与渲染常量 ---
#define DINO_X 20           // 恐龙的固定 X 坐标
#define GROUND_Y 100        // 地面的 Y 坐标
#define DINO_SIZE 16        // 恐龙的长宽
#define CACTUS_W 10         // 仙人掌的宽度
#define CACTUS_H 16         // 仙人掌的高度

#define GRAVITY 1           // 重力加速度
#define JUMP_VELOCITY -9    // 起跳初速度 (负数表示向上)
#define CACTUS_SPEED 4      // 仙人掌向左移动的速度
#define GAME_TICK_MS 30     // 游戏物理帧间隔 (约 33 帧/秒)

// --- 游戏状态变量 ---
static int dinoY = GROUND_Y - DINO_SIZE;
static int dinoVy = 0;
static bool isJumping = false;
static int cactusX = 128;   // 屏幕最右侧外边
static int score = 0;
static bool isGameOver = false;
static unsigned long lastTickTime = 0;

// --- UI 控件指针 ---
static lv_obj_t *dinoObj = nullptr;
static lv_obj_t *cactusObj = nullptr;
static lv_obj_t *groundObj = nullptr;
static lv_obj_t *scoreLabel = nullptr;
static lv_obj_t *gameOverLabel = nullptr;

// ==========================================
// [接口] 兼容层：单独创建页面对象
// ==========================================
lv_obj_t* uiDinoPageCreate() {
    return lv_obj_create(NULL);
}

// ==========================================
// [接口] 初始化小恐龙页面
// ==========================================
void uiDinoPageInit(lv_obj_t *parent) {
    // 这个页面我们需要绝对的像素控制权，所以绝对不使用 Flex 布局
    // 移除默认的滚动条，防止游戏画面被拖动
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    // 1. 地面 (一条简单的横线/方块)
    groundObj = lv_obj_create(parent);
    lv_obj_set_size(groundObj, 128, 2);
    lv_obj_align(groundObj, LV_ALIGN_TOP_LEFT, 0, GROUND_Y);
    lv_obj_set_style_bg_color(groundObj, lv_color_black(), 0);
    lv_obj_set_style_border_width(groundObj, 0, 0);
    lv_obj_set_style_radius(groundObj, 0, 0);

    // 2. 恐龙 (黑色小方块)
    dinoObj = lv_obj_create(parent);
    lv_obj_set_size(dinoObj, DINO_SIZE, DINO_SIZE);
    lv_obj_set_style_bg_color(dinoObj, lv_color_black(), 0);
    lv_obj_set_style_border_width(dinoObj, 0, 0);
    lv_obj_set_style_radius(dinoObj, 2, 0);

    // 3. 仙人掌 (深灰色长条)
    cactusObj = lv_obj_create(parent);
    lv_obj_set_size(cactusObj, CACTUS_W, CACTUS_H);
    lv_obj_set_style_bg_color(cactusObj, lv_color_hex(0x555555), 0);
    lv_obj_set_style_border_width(cactusObj, 0, 0);
    lv_obj_set_style_radius(cactusObj, 2, 0);

    // 4. 分数标签 (右上角)
    scoreLabel = lv_label_create(parent);
    lv_obj_align(scoreLabel, LV_ALIGN_TOP_RIGHT, -4, 4);

    // 5. Game Over 提示 (屏幕中央，默认隐藏)
    gameOverLabel = lv_label_create(parent);
    lv_label_set_text(gameOverLabel, "GAME OVER\nPress OK");
    lv_obj_align(gameOverLabel, LV_ALIGN_CENTER, 0, -20);
    lv_obj_set_style_text_align(gameOverLabel, LV_TEXT_ALIGN_CENTER, 0);

    // 初始化重置游戏状态
    uiDinoPageReset();
}

// ==========================================
// [接口] 重置游戏状态
// ==========================================
void uiDinoPageReset() {
    dinoY = GROUND_Y - DINO_SIZE;
    dinoVy = 0;
    isJumping = false;
    cactusX = 128;
    score = 0;
    isGameOver = false;

    if (gameOverLabel) {
        lv_obj_add_flag(gameOverLabel, LV_OBJ_FLAG_HIDDEN); // 隐藏死亡提示
    }
    
    // 强制刷新一次位置
    if (dinoObj) lv_obj_align(dinoObj, LV_ALIGN_TOP_LEFT, DINO_X, dinoY);
    if (cactusObj) lv_obj_align(cactusObj, LV_ALIGN_TOP_LEFT, cactusX, GROUND_Y - CACTUS_H);
    if (scoreLabel) lv_label_set_text_fmt(scoreLabel, "%d", score);
}

// ==========================================
// [接口] 触发跳跃
// ==========================================
void uiDinoPageJump() {
    // [游戏内部动作不使用 click.wav] 
    // 不调用发声函数，维持静音操作
    if (!isJumping && !isGameOver) {
        dinoVy = JUMP_VELOCITY; // 赋予一个向上的初速度
        isJumping = true;
    }
}

// ==========================================
// [接口] 独立刷新游戏物理引擎 (Tick)
// ==========================================
void uiDinoPageUpdate() {
    if (isGameOver) return; // 死亡状态下停止物理引擎运算

    // 1. 更新恐龙物理状态 (受到重力牵引)
    dinoVy += GRAVITY;
    dinoY += dinoVy;

    // 碰撞地面检测
    if (dinoY >= GROUND_Y - DINO_SIZE) {
        dinoY = GROUND_Y - DINO_SIZE;
        dinoVy = 0;
        isJumping = false;
    }

    // 2. 更新仙人掌位置
    cactusX -= CACTUS_SPEED;
    if (cactusX < -CACTUS_W) {
        cactusX = 128; // 从屏幕右侧重新出现
        score++;       // 成功越过，加一分
        if (scoreLabel) lv_label_set_text_fmt(scoreLabel, "%d", score);
    }

    // 3. 碰撞检测 (AABB 简单矩形相交判定)
    bool overlapX = (cactusX < DINO_X + DINO_SIZE) && (cactusX + CACTUS_W > DINO_X);
    bool overlapY = (dinoY + DINO_SIZE > GROUND_Y - CACTUS_H);

    if (overlapX && overlapY) {
        isGameOver = true;
        if (gameOverLabel) {
            lv_obj_clear_flag(gameOverLabel, LV_OBJ_FLAG_HIDDEN); // 显示死亡提示
        }
    }

    // 4. 将物理数据同步到 UI 渲染层
    if (dinoObj) lv_obj_align(dinoObj, LV_ALIGN_TOP_LEFT, DINO_X, dinoY);
    if (cactusObj) lv_obj_align(cactusObj, LV_ALIGN_TOP_LEFT, cactusX, GROUND_Y - CACTUS_H);
}

// ==========================================
// [接口] 页面数据刷新泵 (满足系统契约)
// ==========================================
void uiDinoPageUpdate(const SensorData &data, const SystemState &state) {
    // 依靠 millis() 进行限帧，避免 `main.cpp` 的 loop 跑得太快导致游戏像开了加速器
    if (millis() - lastTickTime > GAME_TICK_MS) {
        lastTickTime = millis();
        uiDinoPageUpdate(); // 调用小 Update 驱动物理帧
    }
}

// ==========================================
// [接口] 页面销毁清理
// ==========================================
void uiDinoPageDeinit() {
    dinoObj = nullptr;
    cactusObj = nullptr;
    groundObj = nullptr;
    scoreLabel = nullptr;
    gameOverLabel = nullptr;
}

// ==========================================
// [接口] 专属按键事件处理
// ==========================================
int uiDinoPageHandleKey(uint32_t key) {
    // [进入页面时 click.wav 由 ui_manager 触发]
    // 在页面内部遇到任何按键，绝不主动播放音效
    
    if (key == UI_KEY_OK || key == UI_KEY_UP) {
        if (isGameOver) {
            uiDinoPageReset(); // 死亡后按确认/上键复活
        } else {
            uiDinoPageJump();  // 存活时按确认/上键跳跃
        }
    }

    // 维持在当前页面。如果要退出游戏，按下左键 (UI_KEY_LEFT) 即可，
    // 因为 ui_manager 已经在底层统一将其拦截并执行栈弹出回退了。
    return DINO_PAGE;
}