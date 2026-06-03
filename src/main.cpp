#include <Arduino.h>
#include <WiFi.h>

#include "Config.h"
#include "WifiScanner.h"
#include "ChannelAnalysis.h"
#include "TimeManager.h"
#include "CaptivePortal.h"
#include "FirebaseManager.h"

enum Mode {
  MODE_SCANNER,
  MODE_PORTAL
};

Mode currentMode = MODE_SCANNER;

unsigned long lastScan = 0;
const unsigned long scanInterval = 10000;

void printMenu();
void connectToInternet();

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("=== ESP32 AUDITORIA WIFI EDUCACIONAL ===");
  Serial.println("Portal defensivo sem captura de credenciais reais.");
  Serial.println();

  connectToInternet();
  setupTimeManager();
  printMenu();

  currentMode = MODE_SCANNER;
}

void loop() {

  if (currentMode == MODE_SCANNER) {
    if (millis() - lastScan > scanInterval) {
      scanNetworks();
      lastScan = millis();
    }
  }

  if (currentMode == MODE_PORTAL) {
    handleCaptivePortal();
  }

  if (Serial.available()) {
    char cmd = Serial.read();
    switch (cmd) {
      case '1':
        Serial.println("Modo Scanner");
        WiFi.mode(WIFI_STA);
        currentMode = MODE_SCANNER;
        break;

      case '2':
        Serial.println("Modo Portal");
        startCaptivePortal();
        currentMode = MODE_PORTAL;
        break;

      case '3':
        showFoundNetworks();
        break;

      case '4':
        analyzeChannels();
        break;

      case '5':
        connectToInternet();
        break;

      case 'h':
        printMenu();
        break;
    }
  }

  delay(100);
}

void connectToInternet() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao WiFi externo");
  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    Serial.print(".");
    delay(500);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado!");
    Serial.println("IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nNao conectou ao WiFi externo.");
  }
}

void printMenu() {  
  Serial.println("Comandos:");
  Serial.println("1 - Scanner WiFi");
  Serial.println("2 - Portal Captivo Defensivo");
  Serial.println("3 - Mostrar redes encontradas");
  Serial.println("4 - Analise de canais");
  Serial.println("5 - Reconectar WiFi externo");
  Serial.println("h - Mostrar menu");

  Serial.println();
}