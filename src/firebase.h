#pragma once
#include <Arduino.h>
#include "actuators.h"


extern String g_idToken;
extern String g_localId;
extern String DEVICE_NODE;


bool firebaseAnonSignIn();


bool firebasePatchAll();


void firebasePullActuators();
