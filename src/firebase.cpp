#include "firebase.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "sensors.h"
#include "actuators.h"

const char* FIREBASE_API_KEY = "AIzaSyB3iSrRgGOms2TRwAxZbO1Cv7b6wrhldLA";
const char* FIREBASE_DB_URL  = "https://smartfarms-27323-default-rtdb.firebaseio.com";
String g_idToken;
String g_localId;
String DEVICE_NODE;
int lightOnSec = 0;
int lightOffSec = 0;

static bool httpPOST(const String& url, const String& json, String* resp=nullptr) {
    WiFiClientSecure client; client.setInsecure();
    HTTPClient http;
    if (!http.begin(client, url)) return false;
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(json);
    if (resp) *resp = http.getString();
    http.end();
    return (code >= 200 && code < 300);
}

static bool httpPATCH(const String& url, const String& json, String* resp=nullptr) {
    WiFiClientSecure client; client.setInsecure();
    HTTPClient http;
    if (!http.begin(client, url)) return false;
    http.addHeader("Content-Type", "application/json");
    int code = http.sendRequest("PATCH",
                                (uint8_t*)json.c_str(),
                                json.length());
    if (resp) *resp = http.getString();
    http.end();
    return (code >= 200 && code < 300);
}

static bool httpGET(const String& url, String* resp=nullptr) {
    WiFiClientSecure client; client.setInsecure();
    HTTPClient http;
    if (!http.begin(client, url)) return false;
    int code = http.GET();
    if (resp) *resp = http.getString();
    http.end();
    return (code >= 200 && code < 300);
}


bool firebaseAnonSignIn() {
    String url  = "https://identitytoolkit.googleapis.com/v1/accounts:signUp?key=" 
                  + String(FIREBASE_API_KEY);
    String body = "{\"returnSecureToken\":true}";

    String resp;
    if (!httpPOST(url, body, &resp)) {
        Serial.println("[FB] HTTP signUp FAIL");
        return false;
    }

    StaticJsonDocument<1024> doc;
    auto err = deserializeJson(doc, resp);
    if (err) {
        Serial.print("[FB] JSON signUp FAIL: ");
        Serial.println(err.c_str());
        Serial.println(resp);
        return false;
    }

    g_idToken = doc["idToken"].as<String>();
    g_localId = doc["localId"].as<String>();

    Serial.print("[FB] Anon UID: ");
    Serial.println(g_localId);
    return true;
}

// PUSH sensors + actuators 
bool firebasePatchAll() {
    if (DEVICE_NODE == "" || g_idToken.length() < 10) return false;

    String url = String(FIREBASE_DB_URL) + DEVICE_NODE + ".json?auth=" + g_idToken;

    StaticJsonDocument<512> doc;

    doc["ts"] = (uint32_t)(millis() / 1000);

    // Sensors
    JsonObject s = doc.createNestedObject("sensors");
    s["temp"] = isnan(v_temp) ? -1 : v_temp;
    s["humi"] = isnan(v_humi) ? -1 : v_humi;
    s["lux"]  = v_lux;
    s["soil"] = v_soil;

    // Actuators
    JsonObject a = doc.createNestedObject("actuators");
    a["light"] = s_light ? 1 : 0;
    const char* lightStr = (lightMode == LIGHT_MODE_AUTO) ? "auto" : "manual";
    a["lightMode"] = lightStr;

    const char* pumpStr = "off";
    switch (pumpState) {
    case PUMP_MANUAL: pumpStr = "manual"; break;
    case PUMP_AUTO:   pumpStr = "auto";   break;
    default:          pumpStr = "off";    break;
    }
    a["pump"]   = pumpStr;
    a["buzzer"] = s_buzz ? 1 : 0;
    JsonObject cfg = doc.createNestedObject("config");
    cfg["soilThreshold"] = soilThreshold;

    String json; 
    serializeJson(doc, json);

    if (!httpPATCH(url, json)) {
        Serial.println("[FB] PATCH FAIL → thử sign-in lại");
        if (firebaseAnonSignIn()) {
            return httpPATCH(url, json);
        }
        return false;
    }
    // Serial.println("[FB] PATCH OK");
    return true;
}

//  PULL actuators 
void firebasePullActuators() {
    if (DEVICE_NODE == "" || g_idToken.length() < 10) return;

    String url = String(FIREBASE_DB_URL) + DEVICE_NODE + "/actuators.json?auth=" + g_idToken;

    String resp;
    if (!httpGET(url, &resp)) return;

    StaticJsonDocument<512> doc;
    if (deserializeJson(doc, resp)) return;
    if (!doc["lightMode"].isNull()) {
        const char* m = doc["lightMode"];
        if      (!strcmp(m, "auto"))   lightMode = LIGHT_MODE_AUTO;
        else if (!strcmp(m, "manual")) lightMode = LIGHT_MODE_MANUAL;
    }

    if (lightMode == LIGHT_MODE_MANUAL && !doc["light"].isNull()) {
        bool l = doc["light"];
        if (l != s_light) {
            s_light = l;
        }
    }

    // led 0/1 → bool
    if (!doc["light"].isNull()) {
        bool l = doc["light"].as<int>() != 0;
        setLight(l);
    }

    // Buzzer: 0/1 → bool
    if (!doc["buzzer"].isNull()) {
        bool b = doc["buzzer"].as<int>() != 0;
        setBuzzer(b);
    }

    // pump "off" | "manual" | "auto"
    if (!doc["pump"].isNull()) {
        String pm = doc["pump"].as<String>();
        if (pm == "manual")      pumpSetMode(PUMP_MANUAL);
        else if (pm == "auto")   pumpSetMode(PUMP_AUTO);
        else                     pumpSetMode(PUMP_OFF);
    }
}
void firebasePullConfig() {
    if (DEVICE_NODE == "" || g_idToken.length() < 10) return;

    String url = String(FIREBASE_DB_URL) + DEVICE_NODE + "/config.json?auth=" + g_idToken;

    String resp;
    if (!httpGET(url, &resp)) return;

    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, resp)) return;

    if (!doc["soilThreshold"].isNull()) {
        int th = doc["soilThreshold"].as<int>();
        if (th < 0) th = 0;
        if (th > 4095) th = 4095;
        soilThreshold = th;
    }
    if (doc.containsKey("light_on"))  lightOnSec  = doc["light_on"];
    if (doc.containsKey("light_off")) lightOffSec = doc["light_off"];
}