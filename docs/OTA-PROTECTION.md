# Add password protection for OTA updates

## Descrizione

La protezione OTA con password e attualmente opzionale e non implementata nel codice. Per evitare upload malevoli di firmware, e necessario aggiungere una password sicura.

## Cosa fare

- Aggiungere `ArduinoOTA.setPassword()` in `update_fw_manager.cpp`
- La password potrebbe essere definita in `config.h` come gli altri parametri
- Valutare se usare una password fissa o derivata da un hash del device ID

## Riferimento

Funzionalita OTA aggiunta in: `feat: ArduinoOTA per aggiornamenti firmware via rete`
