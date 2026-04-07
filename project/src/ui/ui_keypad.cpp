#include "ui_keypad.h"
#include "../../config.h"

// ==========================================
// Keypad - 5-way joystick driver
// Debounce: 20ms + rising-edge single-fire
// ==========================================

#define DEBOUNCE_DELAY_MS 20
#define STARTUP_IGNORE_MS 500

static uint32_t      s_lastRaw     = UI_KEY_NONE;
static uint32_t      s_stableKey   = UI_KEY_NONE;
static uint32_t      s_reportedKey = UI_KEY_NONE;
static unsigned long s_debounceTs  = 0;
static unsigned long s_ignoreUntil = 0;
static bool          s_armed       = false;

// Read raw GPIO levels. Priority: OK > UP > DOWN > LEFT > RIGHT
static uint32_t readRaw() {
    if (KEY_OK_PIN    != -1 && digitalRead(KEY_OK_PIN)    == LOW) return UI_KEY_OK;
    if (KEY_UP_PIN    != -1 && digitalRead(KEY_UP_PIN)    == LOW) return UI_KEY_UP;
    if (KEY_DOWN_PIN  != -1 && digitalRead(KEY_DOWN_PIN)  == LOW) return UI_KEY_DOWN;
    if (KEY_LEFT_PIN  != -1 && digitalRead(KEY_LEFT_PIN)  == LOW) return UI_KEY_LEFT;
    if (KEY_RIGHT_PIN != -1 && digitalRead(KEY_RIGHT_PIN) == LOW) return UI_KEY_RIGHT;
    return UI_KEY_NONE;
}

// Initialize joystick GPIO pins
bool uiKeypadBegin() {
    if (KEY_UP_PIN    != -1) pinMode(KEY_UP_PIN,    INPUT_PULLUP);
    if (KEY_DOWN_PIN  != -1) pinMode(KEY_DOWN_PIN,  INPUT_PULLUP);
    if (KEY_LEFT_PIN  != -1) pinMode(KEY_LEFT_PIN,  INPUT_PULLUP);
    if (KEY_RIGHT_PIN != -1) pinMode(KEY_RIGHT_PIN, INPUT_PULLUP);
    if (KEY_OK_PIN    != -1) pinMode(KEY_OK_PIN,    INPUT_PULLUP);

    s_lastRaw = UI_KEY_NONE;
    s_stableKey = UI_KEY_NONE;
    s_reportedKey = UI_KEY_NONE;
    s_debounceTs = millis();
    s_ignoreUntil = millis() + STARTUP_IGNORE_MS;
    s_armed = false;

    Serial.println("[Keypad] init ok  UP=38 DOWN=37 LEFT=36 RIGHT=35 OK=47");
    return true;
}

// Read key with 20ms debounce + rising-edge single-fire.
// Returns true ONCE per physical press. Holding does NOT repeat.
bool uiKeypadRead(uint32_t &key) {
    uint32_t raw = readRaw();

    // Debounce: reset timer on any raw change
    if (raw != s_lastRaw) {
        s_debounceTs = millis();
        s_lastRaw    = raw;
    }

    // Stable: commit after DEBOUNCE_DELAY_MS of no change
    if ((millis() - s_debounceTs) >= DEBOUNCE_DELAY_MS) {
        s_stableKey = raw;
    }

    // 上电后先静默一段时间，并要求看到一次稳定释放，
    // 避免摇杆电平抖动在任务刚启动时被当成真实按键。
    if ((long)(millis() - s_ignoreUntil) < 0) {
        key = UI_KEY_NONE;
        return false;
    }

    if (!s_armed) {
        if (s_stableKey == UI_KEY_NONE) {
            s_armed = true;
            s_reportedKey = UI_KEY_NONE;
        }
        key = UI_KEY_NONE;
        return false;
    }

    // Rising edge: new key pressed and not yet reported
    if (s_stableKey != UI_KEY_NONE && s_stableKey != s_reportedKey) {
        s_reportedKey = s_stableKey;
        key = s_stableKey;
        return true;
    }

    // Release: allow next press of same key to fire again
    if (s_stableKey == UI_KEY_NONE) {
        s_reportedKey = UI_KEY_NONE;
    }

    key = UI_KEY_NONE;
    return false;
}
