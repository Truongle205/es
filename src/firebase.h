#pragma once
#include <Arduino.h>
#include "actuators.h"

extern String g_idToken;
extern String g_localId;
extern String DEVICE_NODE;
extern int soilThreshold;
extern String lightOnTime;
extern String lightOffTime;

bool firebaseAnonSignIn();
bool firebasePatchAll();
void firebasePullActuators();
void firebasePullConfig();