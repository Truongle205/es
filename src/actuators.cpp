#include "actuators.h"
#include "sensors.h"
#include <Arduino.h>

// Dùng lại pin trong sensors.h
// PIN_LIGHT, PIN_PUMP, PIN_BUZZER

bool      s_light = false;
bool      s_buzz  = false;
PumpState pumpState = PUMP_OFF;

void actuatorsInit() {
    pinMode(PIN_LIGHT,  OUTPUT);
    pinMode(PIN_PUMP,   OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);

    digitalWrite(PIN_LIGHT,  LOW);
    digitalWrite(PIN_PUMP,   LOW);
    digitalWrite(PIN_BUZZER, LOW);
}

void setLight(bool on) {
    s_light = on;
    digitalWrite(PIN_LIGHT, on ? HIGH : LOW);
}

void setBuzzer(bool on) {
    s_buzz = on;
    digitalWrite(PIN_BUZZER, on ? HIGH : LOW);
}

void pumpSetMode(PumpState st) {
    pumpState = st;
}

// Hàm này chính là state machine cho bơm
void actuatorsStateMachineUpdate() {
    switch (pumpState) {
    case PUMP_OFF:
        digitalWrite(PIN_PUMP, LOW);
        break;

    case PUMP_MANUAL:
        digitalWrite(PIN_PUMP, HIGH);
        break;

    case PUMP_AUTO:
        // v_soil: 0..4095 (đất càng khô càng nhỏ hay càng lớn tùy mạch)
        // Ở đây t giả sử "đất khô" khi ADC < 1500, ông tự chỉnh lại ngưỡng.
        if (v_soil < 1500) {
            digitalWrite(PIN_PUMP, HIGH);
        } else {
            digitalWrite(PIN_PUMP, LOW);
        }
        break;
    }
}
