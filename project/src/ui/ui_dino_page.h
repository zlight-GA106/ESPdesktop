#pragma once
#include <lvgl.h>
#include <stdint.h>

// 前向声明，避免与系统数据总线产生互相包含的头文件灾难
struct SensorData;
struct SystemState;

// ==========================================
// 🦖 恐龙小游戏彩蛋页面接口 (Dino Game Page)
// 职责：实现一个可以通过物理按键游玩的微型跑酷游戏
// ==========================================

// 创建小恐龙页面内容对象 (满足你明确要求的声明)
// [返回值]：返回该页面的底层容器对象指针
lv_obj_t* uiDinoPageCreate();

// 独立刷新游戏物理引擎与画面帧 (满足你明确要求的声明)
// [作用]：作为游戏的主循环 Tick。处理重力下坠、障碍物左移、碰撞检测和分数累加。
void uiDinoPageUpdate();

// 触发恐龙跳跃动作 (满足你明确要求的声明)
// [作用]：当用户按下“确认”或“上”键时调用。赋予恐龙一个向上的初速度，对抗重力。
void uiDinoPageJump();

// 重置游戏状态 (满足你明确要求的声明)
// [作用]：当恐龙撞到仙人掌死亡后，重新初始化恐龙位置、清空障碍物、分数归零。
void uiDinoPageReset();

// ==========================================
// ⚠️ UI 管理器生命周期与路由契约
// 为了配合 ui_manager.cpp 中已经写死的压栈路由机制，必须补充以下标准接口。
// 注意：此处的 Update 同样构成了 C++ 的函数重载 (Overload)。
// ==========================================

// 初始化小恐龙页面
// [参数]：parent - ui_manager 提供的内容区容器 (frame.content)
void uiDinoPageInit(lv_obj_t *parent);

// 销毁小恐龙页面
// [作用]：在页面退出时安全释放内部控件指针，并停止游戏的内部定时器或逻辑。
void uiDinoPageDeinit();

// 刷新页面全局数据 (满足 ui_manager 统一调用的契约)
// [作用]：全局主循环高频调用。内部可直接转调无参的 uiDinoPageUpdate() 来驱动游戏帧率。
void uiDinoPageUpdate(const SensorData &data, const SystemState &state);

// 处理小恐龙页面的按键事件
// [参数]：key - 摇杆按键枚举
// [返回值]：返回期望跳转的目标页面 ID。拦截跳跃/重置按键，其余交由全局返回拦截。
int uiDinoPageHandleKey(uint32_t key);