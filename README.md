# ESPControlBase

Firmware generico per dispositivi ESP8266 domestici. Supporta sensori, display OLED, relay, LED RGB e comunicazione MQTT con comandi remoti e telemetria periodica. Progettato come base riutilizzabile per nodi domotici.

---

## Features

- **Sensore GY-21 (Si7021)** — lettura temperatura e umidita via I2C
- **Display OLED SSD1306 128x64** — mostra IP, stato MQTT, temperatura, umidita e stato attuatori con auto-off dopo timeout configurabile
- **Relay** — controllo remoto con supporto a relay bistabile o momentaneo (pulse configurabile)
- **LED RGB** — Grove Chainable LED con animazione a colori
- **MQTT** — publish periodico dello status, risposta immediata dopo ogni comando, announce al boot, LWT per monitoring connessione
- **HTTP legacy** — REST API di fallback su porta 80
- **Pulsante flash** — attivazione display con debounce software

---

## Hardware

| Componente | Modello | Interfaccia |
|---|---|---|
| Microcontrollore | ESP8266 ESP-12E | — |
| Sensore temp/hum | GY-21 (Si7021) | I2C (0x40) |
| Display | SSD1306 OLED 128x64 | I2C (0x3C) |
| Relay | Modulo relay 5V | Digitale (D1) |
| LED | Grove Chainable RGB | Seriale (D2/D7) |
| Pulsante | Flash integrato ESP8266 | Digitale (D3) |

---

## Wiring / Pinout

| Pin ESP8266 | GPIO | Funzione |
|---|---|---|
| D1 | GPIO5 | Relay (active HIGH) |
| D2 | GPIO4 | LED clock |
| D3 | GPIO0 | Pulsante flash (INPUT_PULLUP, LOW = premuto) |
| D4 | GPIO2 | LED blu status onboard (active LOW) |
| D5 | GPIO14 | I2C SDA |
| D6 | GPIO12 | I2C SCL |
| D7 | GPIO13 | LED data |

---

## Configurazione

Tutte le impostazioni sono in `include/config.h`. Nessuna modifica al codice sorgente necessaria.

### Identita dispositivo

```cpp
#define DEVICE_ID       "esp_control_base"   // ID usato nei topic MQTT
#define DEVICE_NAME     "ESPControlBase"     // Nome leggibile
#define DEVICE_TYPE     "generic"            // Tipo di device
```

### WiFi

```cpp
#define WIFI_SSID       "LaTuaRete"         // SSID della rete WiFi
#define WIFI_PSW        "password"          // Password WiFi
```

### MQTT

```cpp
#define MQTT_HOST       "raspberrypi.local"  // Host del broker MQTT
#define MQTT_PORT       1883                 // Porta del broker
#define MQTT_TOPIC_PREFIX "guiver"           // Prefisso topic (guiver/{id}/...)
#define STATUS_INTERVAL 5                    // Secondi tra publish status
```

Il topic prefix e usato in tutti i topic MQTT: `{prefix}/{device_id}/status`, `{prefix}/{device_id}/command`, ecc.

### Relay

```cpp
#define RELAY_NAME      "pompa"             // Nome del relay (display, MQTT, JSON)
#define RELAY_CMD       "set_" RELAY_NAME   // Command name (auto-derivato)
#define RELAY_TYPE      RELAY_BISTABLE      // RELAY_BISTABLE (0) o RELAY_MOMENTARY (1)
#define RELAY_PULSE_MS  500                 // Durata impulso in ms (solo MOMENTARY)
```

`RELAY_NAME` determina:
- La **chiave JSON** nello status e nella response MQTT
- Il **label** nell'announce MQTT
- Il **testo** mostrato sul display OLED
- Il **command name** (`"set_" + RELAY_NAME`)

Esempio con `#define RELAY_NAME "luce"`:

| Elemento | Risultato |
|---|---|
| Status JSON | `{"temperature": 23.5, "humidity": 45.2, "luce": true}` |
| Command | `{"cmd": "set_luce", "value": true}` |
| Display | `luce: ON` |
| Announce | `{"name": "set_luce", "label": "luce"}` |

### Display

```cpp
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
#define SCREEN_ADDRESS  0x3C
#define DISPLAY_TIMEOUT 5          // Secondi prima dello spegnimento auto
```

### Abilitazione moduli

```cpp
#define ENABLE_DISPLAY      1     // 0 = display disabilitato
#define ENABLE_SENSOR_GY21  1     // 0 = sensore GY-21 disabilitato
#define ENABLE_LED          1     // 0 = LED RGB disabilitato
#define ENABLE_RELAY        1     // 0 = relay disabilitato
```

Impostare a `0` per disabilitare un modulo — il codice relativo viene escluso a compile-time.

---

## Build & Upload

Richiede [PlatformIO](https://platformio.org/).

```bash
pio run                  # build
pio run -t upload        # build + upload via seriale
pio run -t monitor       # seriale monitor (9600 baud)
pio run -e esp12e_ota -t upload   # build + upload via WiFi (OTA, senza USB)
```

L'upload remoto (`esp12e_ota`) usa ArduinoOTA sulla porta 8266 verso l'IP in `platformio.ini` (`upload_port`) con autenticazione (`--auth`, stessa password di `OTA_PASSWORD` in `include/config.h`). La scheda deve essere accesa e con firmware funzionante, sulla stessa rete. Il primo flash da scheda vergine o firmware non avviabile va fatto via seriale tenendo premuto il tasto FLASH all'accensione (nessun auto-reset cablato).

Se l'IP della scheda cambia (DHCP), aggiornare `upload_port` nell'env `esp12e_ota`.

---

## Documentazione

- [MQTT — Protocollo, topic, comandi e esempi](docs/MQTT.md)
- [AGENTS.md — Info sviluppo, struttura progetto, gotchas](AGENTS.md)
