#include "light_sensor.h"
#include "../../config.h"
#include <Wire.h>
#include <BH1750.h>

// ==========================================
// ☀️ 光照强度传感器驱动实现 (BH1750)
// ==========================================

// 实例化 BH1750 对象，传入 config.h 中定义的 I2C 地址 (默认 0x23)
BH1750 lightMeter(LIGHT_SENSOR_ADDR);

// 局部状态标志，记录传感器是否成功上线
static bool isLightSensorConnected = false;

// ==========================================
// [流程] 初始化逻辑
// ==========================================
bool lightSensorBegin() {
#if FAKE_DATA_MODE
    // [假数据模式] 跳过硬件初始化
    Serial.println("[DEBUG] BH1750 光照传感器 (假数据模式)");
    isLightSensorConnected = true;
    return true;
#else
    // 1. 确保 I2C 总线已启动。
    // 虽然温湿度模块可能已经调过 Wire.begin()，但为了模块绝对解耦，
    // 这里再调用一次是安全的（ESP32 底层会忽略重复的同引脚初始化）。
    Wire.begin(I2C_SDA, I2C_SCL);

    // 2. 尝试启动传感器
    // 使用 CONTINUOUS_HIGH_RES_MODE (连续高分辨率模式)，测量精度为 1 Lux，测量时间约 120ms
    if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, LIGHT_SENSOR_ADDR, &Wire)) {
        Serial.println("[成功] BH1750 光照传感器初始化完毕");
        isLightSensorConnected = true;
        return true;
    } else {
        Serial.println("[错误] 找不到 BH1750 传感器，请检查 SDA(18)/SCL(8) 或地址引脚！");
        isLightSensorConnected = false;
        return false;
    }
#endif
}

// ==========================================
// [流程] 读取处理后的物理值 (Lux)
// ==========================================
float lightSensorRead() {
    if (!isLightSensorConnected) {
        return -1.0f; // -1 表示硬件离线或出错
    }

#if FAKE_DATA_MODE
    // [假数据模式] 生成虚拟光照数据，模拟日光变化
    static unsigned long lastFakeTime = 0;
    static float fakeLux = 200.0f;
    unsigned long now = millis();
    
    // 每 5000ms 改变一次光照强度，模拟日外光变化
    if (now - lastFakeTime > 5000) {
        lastFakeTime = now;
        // 在 50-800 Lux 之间变化，模拟室内不同光线条件
        fakeLux = 100.0f + random(0, 701);
    }
    return fakeLux;
#else
    // 从传感器读取光照度
    // [原始值和处理值的区别]：
    // 如果使用普通光敏电阻(ADC方案)，读出来的是 0-4095 的原始电压值(Raw)，
    // 需要极其复杂的非线性数学拟合公式才能转换成勒克斯(Lux)。
    // 但 BH1750 是全数字芯片，内部已经完成了光子积分和 ADC 转换，
    // 它吐出来的就是经过光学标准校准的最终处理值 (Lux)，所以我们无需再写 Raw 读取接口。
    float lux = lightMeter.readLightLevel();
    
    // 如果读取失败，库通常会返回负数（如 -1.0 或 -2.0）
    if (lux < 0.0f) {
        return -1.0f;
    }
    
    return lux;
#endif
}