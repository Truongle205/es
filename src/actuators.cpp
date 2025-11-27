#include "firebase.h"
#include "actuators.h"
#include "sensors.h"
#include <Arduino.h>
  

bool      s_light = false;
bool      s_buzz  = false;
PumpState pumpState = PUMP_OFF;
int soilThreshold = 1500;
extern bool s_light;

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

void actuatorsStateMachineUpdate(unsigned long nowSec) {
    switch (pumpState) {
    case PUMP_OFF:
        digitalWrite(PIN_PUMP, LOW);
        break;

    case PUMP_MANUAL:
        digitalWrite(PIN_PUMP, HIGH);
        break;

    case PUMP_AUTO:
        if (v_soil < soilThreshold) {
            digitalWrite(PIN_PUMP, HIGH);
        } else {
            digitalWrite(PIN_PUMP, LOW);
        }
        break;
    }
    if (lightOnSec > 0 && lightOffSec > 0 && lightOffSec > lightOnSec) {

        if (nowSec >= (unsigned long)lightOnSec &&
            nowSec <  (unsigned long)lightOffSec)
        {
            if (!s_light) {
                s_light = true;
                digitalWrite(PIN_LIGHT, HIGH);
                Serial.println("[LIGHT] BẬT theo config");
            }
        }
        else {
            if (s_light) {
                s_light = false;
                digitalWrite(PIN_LIGHT, LOW);
                Serial.println("[LIGHT] TẮT theo config");
            }
        }
    }
}
