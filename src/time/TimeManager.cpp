#include "../time/TimeManager.h"

#include <WiFiUdp.h>
#include <WiFi.h>
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
    if (WiFi.status() == WL_CONNECTED) {
        timeClient.update();
    }
    return timeClient.getFormattedTime();
}