#pragma once
#include <Arduino.h>
extern String fleetControllerMac;
void fleetSetup(const char* variant);
bool fleetService(void (*stopMotors)());
bool fleetInhibited();
