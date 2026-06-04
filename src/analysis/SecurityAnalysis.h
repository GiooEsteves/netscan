#ifndef SECURITY_ANALYSIS_H
#define SECURITY_ANALYSIS_H

#include <Arduino.h>
#include "Models.h"

extern Alert alerts[30];
extern int alertCount;

void generateSecurityAlerts();
bool hasOpenNetwork();
bool detectEvilTwin();
String getEvilTwinJson();
WiFiQuality calculateWiFiQuality();

#endif