#include "wifi_setup.h"
#include <WiFi.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include "esp_wifi.h"

bool wifiConnect() {
  Serial.println("[WiFi] Starting WiFi setup...");

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);  
  WiFi.persistent(true);       
  esp_wifi_set_ps(WIFI_PS_NONE);

  WiFiManager wm;
  wm.setConfigPortalTimeout(180);   
  bool ok = wm.autoConnect("FARM-SETUP");
  if (!ok) {
    Serial.println("[WiFi] Connect FAIL");
    return false;
  }

  Serial.print("[WiFi] OK, IP: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("espfarm")) {
    Serial.println("[mDNS] http://espfarm.local");
  }
  return true;
}
