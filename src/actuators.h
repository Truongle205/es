#pragma once
#include <Arduino.h>
#include "sensors.h"

enum LightMode {
    LIGHT_MODE_MANUAL = 0,
    LIGHT_MODE_AUTO   = 1
};

enum PumpState {
    PUMP_OFF = 0,
    PUMP_MANUAL,
    PUMP_AUTO
};

extern bool      s_light;
extern bool      s_buzz;
extern PumpState pumpState;
extern LightMode lightMode;

void actuatorsInit();

void setLight(bool on);
void setBuzzer(bool on);


void pumpSetMode(PumpState st);

void actuatorsStateMachineUpdate(unsigned long nowSec);
