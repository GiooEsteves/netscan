# NETSCAN - Sistema IoT de Auditoria Wi-Fi com ESP32

O NetScan é um projeto desenvolvido com ESP32 voltado para análise, monitoramento e demonstração de conceitos de segurança em redes sem fio (Wi-Fi).

## Funcionalidades
- Escaneia redes wi-fi próximas
- Analisa segurança das redes wi-fi
- Analisa congestionamento dos canais wi-fi
- Cria portal de auditoria de rede
- Fornece dashboard web
- Registra logs no Firebase
- Fornece APIs JSON
- Exportação de relatórios CSV

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
│   ├── Models.h
│   └── Config.h
│   │
│   ├── analysis/
│   |   ├── ChannelAnalysis.cpp
│   |   ├── ChannelAnalysis.h
│   |   ├── SecurityAnalysis.cpp
│   |   └── SecurityAnalysis.h
│   │
│   ├── api/
│   |   ├── ApiRoutes.cpp
│   |   └── ApiRoutes.h
│   │
│   ├── firebase/
│   |   ├── FirebaseManager.cpp
│   |   └── FirebaseManager.h
│   │
│   ├── portal/
│   |   ├── CaptivePortal.cpp
│   |   └── CaptivePortal.h
│   │
│   ├── scanner/
│   |   ├── WifiScanner.cpp
│   |   └── WifiScanner.h
│   │
│   ├── time/
│   |   ├── TimeManager.cpp
│   |   └── TimeManager.h
│
├── platformio.ini
│
└── README.md
```

## Como Funciona?
O NetScan IoT funciona utilizando um ESP32 como dispositivo principal para escaneamento, análise e monitoramento de redes Wi-Fi em tempo real.

O sistema opera em dois modos simultaneamente:

* **Station Mode (STA)** → mantém o ESP32 conectado à internet
* **Access Point Mode (AP)** → cria uma rede Wi-Fi própria para acesso ao portal web

Para isso, o ESP32 utiliza o modo:

```cpp
WIFI_AP_STA
```

Esse funcionamento permite que o dispositivo realize auditorias das redes próximas enquanto disponibiliza um dashboard web acessível diretamente pelo navegador.

### Inicialização do Sistema

Ao iniciar, o ESP32:

1. Inicializa o monitor serial
2. Conecta-se ao Wi-Fi externo
3. Sincroniza horário utilizando NTP
4. Exibe o menu de comandos no terminal serial
5. Inicializa os módulos internos do sistema

O usuário pode controlar o sistema através do monitor serial utilizando comandos numéricos.


### Scanner de Redes Wi-Fi

O sistema realiza escaneamento das redes próximas utilizando a biblioteca WiFi do ESP32.

Durante o processo de scan, são coletadas informações como:

* SSID da rede
* Intensidade do sinal (RSSI)
* Canal Wi-Fi
* Endereço MAC (BSSID)
* Tipo de criptografia
* Horário da detecção

Essas informações são armazenadas para posterior análise e exibição no dashboard.

### Sistema de Análise

Após o escaneamento, o projeto executa análises automáticas sobre o ambiente wireless.

#### Análise de Canais

O sistema identifica:

* Quantidade de redes por canal
* Canais congestionados
* Canal mais recomendado para utilização

#### Qualidade do Ambiente Wi-Fi

O sistema calcula um score de qualidade da rede baseado em:

* Redes abertas
* Congestionamento
* Intensidade do sinal
* Quantidade de redes próximas
* Possíveis Evil Twins

O resultado é exibido como:

* Boa
* Média
* Ruim

#### Sistema de Alertas

O projeto gera alertas automáticos para situações como:

* Redes abertas detectadas
* Canais congestionados
* Sinais excessivamente fortes
* Possíveis pontos de acesso suspeitos

Esses alertas são exibidos no dashboard em tempo real.

#### Detecção de Evil Twin

O sistema compara redes com:

* Mesmo SSID
* BSSID diferente

Essa situação pode indicar um possível ataque de Evil Twin, onde um ponto de acesso falso tenta se passar por uma rede legítima.

### Portal de Auditoria
O ESP32 cria um Access Point próprio que hospeda um portal web local.

Ao conectar-se à rede do ESP32, o usuário pode acessar:

```text
http://192.168.4.1
```

O portal possui:

* Tela de login
* Dashboard em tempo real
* Alertas
* Gráficos
* APIs REST
* Exportação de relatórios

### Dashboard Web
O dashboard foi desenvolvido utilizando:

* HTML
* CSS
* JavaScript

As informações são atualizadas automaticamente em tempo real utilizando requisições periódicas para as APIs do ESP32.

O painel apresenta:

* Quantidade de redes detectadas
* Canal recomendado
* Score de qualidade Wi-Fi
* Alertas de segurança
* Redes detectadas
* Intensidade de sinal
* Distribuição dos canais

### APIs REST

O sistema disponibiliza APIs organizadas para acesso aos dados:

```text
/api/networks
/api/channels
/api/stats
/api/alerts
/api/quality
/api/evil-twin
/api/report/csv
```

As APIs retornam dados em JSON ou CSV para integração com dashboards, aplicações externas ou exportação de relatórios.

### Exportação de Relatórios

O sistema permite exportar relatórios contendo:

* Redes detectadas
* Intensidade do sinal
* Canais utilizados
* Criptografia
* Horário da auditoria

Os relatórios podem ser exportados em formato CSV.


### Integração com Firebase

Quando conectado à internet, o ESP32 pode enviar:

* Logs
* Eventos
* Auditorias
* Redes detectadas

para o Firebase, permitindo armazenamento remoto das informações.
