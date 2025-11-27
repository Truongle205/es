#pragma once
#include <Arduino.h>
#include "actuators.h"


extern String g_idToken;
extern String g_localId;
extern String DEVICE_NODE;
extern int soilThreshold;
extern int lightOnSec;
extern int lightOffSec;

bool firebaseAnonSignIn();


bool firebasePatchAll();


void firebasePullActuators();
void firebasePullConfig(); 