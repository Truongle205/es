#include "sensors.h"
#include <Arduino.h>
#include <DHT.h>

DHT dht(PIN_DHT, DHTTYPE);

float v_temp = NAN;
float v_humi = NAN;
int   v_lux  = 0;
int   v_soil = 0;

void sensorsInit() {
    dht.begin();
    analogReadResolution(12);
}

void readSensors() {
    v_temp = dht.readTemperature();
    v_humi = dht.readHumidity();
    v_lux  = analogRead(PIN_LDR_ADC);
    v_soil = analogRead(PIN_SOIL_ADC);
}