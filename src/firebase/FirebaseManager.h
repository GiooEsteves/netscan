#ifndef FIREBASE_MANAGER_H
#define FIREBASE_MANAGER_H

#include "../Models.h"

void sendNetworkToFirebase(NetworkInfo network);
void sendAuditLogToFirebase(String action, String detail);

#endif