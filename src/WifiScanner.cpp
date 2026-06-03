#include "WifiScanner.h"
#include "TimeManager.h"
#include "FirebaseManager.h"
#include <WiFi.h>

NetworkInfo networks[50];
int networkCount = 0;
int channelUsage[14];

String getEncryptionName(uint8_t type) {
  switch (type) {
    case WIFI_AUTH_OPEN: return "ABERTA";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA";
    case WIFI_AUTH_WPA2_PSK: return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2 Enterprise";
    case WIFI_AUTH_WPA3_PSK: return "WPA3";
    default: return "DESCONHECIDA";
  }
}

void scanNetworks() {
  Serial.println("Escaneando redes WiFi...");

  for (int i = 0; i < 14; i++) channelUsage[i] = 0;

  int n = WiFi.scanNetworks();
  networkCount = 0;

  for (int i = 0; i < n && networkCount < 50; i++) {
    networks[networkCount].ssid = WiFi.SSID(i);
    networks[networkCount].rssi = WiFi.RSSI(i);
    networks[networkCount].bssid = WiFi.BSSIDstr(i);
    networks[networkCount].channel = WiFi.channel(i);
    networks[networkCount].encryption = getEncryptionName(WiFi.encryptionType(i));
    networks[networkCount].detectedAt = getCurrentDateTime();

    int ch = networks[networkCount].channel;
    if (ch >= 1 && ch <= 13) channelUsage[ch]++;

    Serial.printf("%d | %s | RSSI %d | Canal %d | %s\n",
      networkCount + 1,
      networks[networkCount].ssid.c_str(),
      networks[networkCount].rssi,
      networks[networkCount].channel,
      networks[networkCount].encryption.c_str()
    );

    sendNetworkToFirebase(networks[networkCount]);

    networkCount++;
  }

  WiFi.scanDelete();
}

void showFoundNetworks() {
  Serial.println("=== REDES ENCONTRADAS ===");

  if (networkCount == 0) {
    Serial.println("Nenhuma rede encontrada ainda.");
    return;
  }

  for (int i = 0; i < networkCount; i++) {
    Serial.println("------------------------");
    Serial.println("SSID: " + networks[i].ssid);
    Serial.println("BSSID: " + networks[i].bssid);
    Serial.println("RSSI: " + String(networks[i].rssi));
    Serial.println("Canal: " + String(networks[i].channel));
    Serial.println("Seguranca: " + networks[i].encryption);
    Serial.println("Horario: " + networks[i].detectedAt);
  }
}