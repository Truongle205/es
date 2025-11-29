#include <Arduino.h>
#include <WiFi.h>
#include "wifi_setup.h"
#include "firebase.h"
#include "sensors.h"
#include "actuators.h"
#include <WiFiManager.h>
#include <time.h>

unsigned long tSense=0, tPush=0, tPull=0, tConfig=0;
unsigned long lastTick = 0;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;
const int   daylightOffset_sec = 0;

void initNTP() {
  Serial.print("[NTP] Configuring time server...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  time_t now = time(nullptr);
  int attempt = 0;
  while (now < 10000 && attempt < 10) {
    delay(500);
    now = time(nullptr);
    attempt++;
    Serial.print(".");
  }

  struct tm timeinfo;
  if (now > 10000) {
    localtime_r(&now, &timeinfo);
    Serial.printf("\n[NTP] Time synchronized: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  } else {
    Serial.println("\n[NTP] Time synchronization FAILED.");
  }
}

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
  
  initNTP();  

  String mac = WiFi.macAddress();
  mac.replace(":", "");
  DEVICE_NODE = "/devices/" + mac;

  if (!firebaseAnonSignIn()) {
    Serial.println("[FB] signin FAIL → restart");
    delay(1500); ESP.restart();
  }
  while(1){
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
}

void loop() {
}