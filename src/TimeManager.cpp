#include "TimeManager.h"

#include <WiFiUdp.h>
#include <NTPClient.h>

WiFiUDP ntpUDP;

NTPClient timeClient(
    ntpUDP,
    "pool.ntp.org",
    -10800,
    60000
);

void setupTimeManager() {
    timeClient.begin();
    timeClient.update();
}

String getCurrentDateTime() {
    timeClient.update();
    return timeClient.getFormattedTime();
}