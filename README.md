# NETSCAN - Sistema IoT de Auditoria Wi-Fi com ESP32

O NetScan é um projeto desenvolvido com ESP32 voltado para análise, monitoramento e demonstração de conceitos de segurança em redes sem fio (Wi-Fi).

## Funcionalidades
- Escaneia redes wi-fi próximas
- Analisa segurança das redes wi-fi
- Analisa congestionamento dos canais wi-fi
- Cria portal captivo
- Fornece dashboard web
- Registra logs no Firebase
- Fornece APIs JSON

## Tecnologias Utilizadas
### Hardware
- ESP32 DevKit V1
- Módulo Wi-Fi integrado
- Fonte USB

### Software
- C++
- Extensão PlatformIO
- Arduino Framework
- HTML/CSS/JavaScript
- ESPAsyncWebServer
- WiFi.h
- DNSServer

## Arquitetura
```txt
netscan-iot/
│
├── src/
│   ├── main.cpp
│   ├── WifiScanner.cpp
│   ├── CaptivePortal.cpp
│   ├── ChannelAnalysis.cpp
│   └── TimeManager.cpp
│
├── include/
│   ├── WifiScanner.h
│   ├── CaptivePortal.h
│   ├── ChannelAnalysis.h
│   ├── Models.h
│   ├── Config.h
│   └── TimeManager.h
│
├── platformio.ini
│
├── docs/
│   ├── imagens/
│   ├── diagramas/
│   └── relatorio.pdf
│
└── README.md
```

## Como Funciona?
