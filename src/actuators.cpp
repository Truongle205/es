#include "firebase.h"
#include "actuators.h"
#include "sensors.h"
#include <Arduino.h>
#include <time.h>

bool      s_light = false;
bool      s_buzz  = false;
PumpState pumpState = PUMP_OFF;
int soilThreshold = 1500;
LightMode lightMode = LIGHT_MODE_AUTO;

int getNowMinutes() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return -1;
    return timeinfo.tm_hour * 60 + timeinfo.tm_min;
}

int timeStringToMinutes(const String& timeStr) {
    if (timeStr.length() != 5 || timeStr.charAt(2) != ':') return -1;
    int hour = timeStr.substring(0, 2).toInt();
    int minute = timeStr.substring(3, 5).toInt();
    if (hour >= 0 && hour < 24 && minute >= 0 && minute < 60) return hour * 60 + minute;
    return -1;
}

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
            digitalWrite(PIN_PUMP, (v_soil < soilThreshold) ? HIGH : LOW);
            break;
    }

    if (lightMode == LIGHT_MODE_AUTO) {
        int nowMins = getNowMinutes();
        int onMins  = timeStringToMinutes(lightOnTime);
        int offMins = timeStringToMinutes(lightOffTime);

        if (nowMins != -1 && onMins != -1 && offMins != -1) {
            bool shouldOn;

            if (onMins < offMins)
                shouldOn = (nowMins >= onMins && nowMins < offMins);
            else
                shouldOn = (nowMins >= onMins || nowMins < offMins);

            if (shouldOn != s_light) {
                setLight(shouldOn);
                Serial.printf("[LIGHT] AUTO -> %s (Now: %d Mins | Schedule: %s-%s)\n",
                              shouldOn ? "ON" : "OFF",
                              nowMins,
                              lightOnTime.c_str(),
                              lightOffTime.c_str());
            }
        } else {
            if (nowMins == -1)
                Serial.println("[LIGHT] AUTO mode: NTP time not available.");
            else
                Serial.printf("[LIGHT] AUTO invalid config (On=%d, Off=%d)\n", onMins, offMins);
        }
    } else {
        digitalWrite(PIN_LIGHT, s_light ? HIGH : LOW);
    }
}
