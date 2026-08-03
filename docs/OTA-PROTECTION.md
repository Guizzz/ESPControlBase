# Password protection for OTA updates

## Stato: implementata

La protezione OTA con password e attiva nel firmware.

## Come funziona

- `ArduinoOTA.setPassword()` viene chiamato in `update_fw_manager.cpp` usando `OTA_PASSWORD` da `include/config.h`
- L'ambiente `esp12e_ota` in `platformio.ini` passa la stessa password a espota tramite `--auth`
- Senza `--auth` l'upload OTA viene rifiutato dal dispositivo (`OTA_AUTH_ERROR`)

## Configurazione

La password e unica in `config.h`:

```cpp
#define OTA_PASSWORD "YOUR_OTA_PASSWORD"
```

Va mantenuta identica a `upload_flags` in `platformio.ini`:

```ini
[env:esp12e_ota]
extends = esp12e
upload_protocol = espota
upload_port = 192.168.1.147
upload_flags =
    --port=8266
    --auth=YOUR_OTA_PASSWORD
```

## Note

- `OTA_PASSWORD` e definito via `#ifdef` in `update_fw_manager.cpp`: se non definito, la protezione e disattivata
- Trattare `config.h` come file sensibile (contiene gia credenziali WiFi/MQTT)

## Riferimento

Funzionalita OTA aggiunta in: `feat: ArduinoOTA per aggiornamenti firmware via rete`
