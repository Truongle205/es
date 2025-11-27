#include <Arduino.h>
#include <WiFi.h>
#include "wifi_setup.h"
#include "firebase.h"
#include "sensors.h"
#include "actuators.h"
#include <WiFiManager.h>

unsigned long tSense=0, tPush=0, tPull=0, tConfig=0;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("ESP is OK");

  actuatorsInit();
  sensorsInit();

  if (!wifiConnect()) {
    Serial.println("WiFi FAILED → reboot in 5s...");
    delay(5000);
    ESP.restart();
  }

  // set DEVICE_NODE = MAC
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  DEVICE_NODE = "/devices/" + mac;

  if (!firebaseAnonSignIn()) {
    Serial.println("[FB] signin FAIL → restart");
    delay(1500); ESP.restart();
  }
}
unsigned long lastTick = 0;
void loop() {
    unsigned long now = millis();

    if (now - tSense >= 2000) { tSense = now; readSensors(); }
    if (now - tPush  >= 5000) { tPush  = now; firebasePatchAll(); }
    if (now - tPull  >= 3000) { tPull  = now; firebasePullActuators(); }
    if (now - tConfig>=15000) { tConfig= now; firebasePullConfig(); }
    if (now - lastTick >= 1000) {
        lastTick = now;
        unsigned long nowSec = now / 1000;
        actuatorsStateMachineUpdate(nowSec);
    }
}
