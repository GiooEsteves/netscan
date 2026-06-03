#include "ChannelAnalysis.h"
#include "WifiScanner.h"
#include "FirebaseManager.h"
#include <Arduino.h>

int getBestChannel() {
  int bestChannel = 1;
  int lowest = 999;

  for (int i = 1; i <= 13; i++) {
    if (channelUsage[i] < lowest) {
      lowest = channelUsage[i];
      bestChannel = i;
    }
  }

  return bestChannel;
}

void analyzeChannels() {
  Serial.println("=== ANALISE DE CANAIS ===");

  for (int i = 1; i <= 13; i++) {
    Serial.printf("Canal %d: %d redes\n", i, channelUsage[i]);
  }

  int best = getBestChannel();

  Serial.println("Canal sugerido: " + String(best));

  sendAuditLogToFirebase("canal_sugerido", String(best));
}