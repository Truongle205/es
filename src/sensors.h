#pragma once
#include "DHT20.h"
#include <Adafruit_AHTX0.h>

#define PIN_DHT       4
#define DHTTYPE       DHT11

#define PIN_LDR_ADC   1
#define PIN_SOIL_ADC  2

#define PIN_LIGHT     48
#define PIN_PUMP      6
#define PIN_BUZZER    18

extern float v_temp, v_humi;
extern int v_lux, v_soil;

void sensorsInit();
void readSensors();