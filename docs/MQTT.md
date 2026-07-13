# MQTT — ESPControlBase

Documentazione del protocollo MQTT utilizzato da ESPControlBase per comunicare con il broker.

> Per la configurazione del device (WiFi, MQTT, relay, pin) vedere il [README](../README.md).

---

## Topic scheme

Tutti i topic sono prefissati con `{prefix}/{device_id}/`, dove `prefix` e `device_id` sono definiti in `config.h` (default: `guiver/esp_control_base`).

| Topic | Direzione | Retained | Scopo |
|---|---|---|---|
| `guiver/{id}/announce` | device → broker | si | Identita del device al boot |
| `guiver/{id}/online` | device → broker | si | Stato connessione (LWT) |
| `guiver/{id}/status` | device → broker | no | Telemetria periodica |
| `guiver/{id}/command` | broker → device | no | Comandi in ingresso |
| `guiver/{id}/response` | device → broker | no | Risposta a un comando |

---

## Announce

Pubblicato una volta al primo collegamento MQTT. Contiene la lista di sensori, attuatori e l'intervallo di polling.

```json
{
  "type": "generic",
  "name": "ESPControlBase",
  "sensors": ["temperature", "humidity"],
  "actuators": [
    {"name": "set_pompa", "label": "pompa"},
    {"name": "set_led",   "label": "set_led"}
  ],
  "interval": 5
}
```

| Campo | Tipo | Descrizione |
|---|---|---|
| `type` | string | Tipo di device |
| `name` | string | Nome leggibile del device |
| `sensors` | array | Lista dei sensori disponibili |
| `actuators[].name` | string | Command name da usare nei messaggi `command` |
| `actuators[].label` | string | Nome leggibile per display/interfaccia |
| `interval` | int | Secondi tra ogni publish di status |

---

## Online / LWT

- Al collegamento: `guiver/{id}/online` → `"1"` (retained)
- Last Will and Testament: se il device si disconnette, il broker pubblica automaticamente `"0"` sullo stesso topic (retained, QOS 1)

---

## Status (periodico)

Pubblicato ogni `STATUS_INTERVAL` secondi (default: 5) e immediatamente dopo ogni comando eseguito.

```json
{
  "temperature": 23.5,
  "humidity": 45.2,
  "pompa": true,
  "led": false
}
```

| Campo | Tipo | Descrizione |
|---|---|---|
| `temperature` | float | Temperatura in gradi Celsius |
| `humidity` | float | Umidita in percentuale |
| `{RELAY_NAME}` | bool | Stato del relay (`true` = acceso) |
| `led` | bool | Stato del LED (`true` = acceso) |

> **Nota:** La chiave del relay dipende dal valore di `RELAY_NAME` in `config.h`. Se `RELAY_NAME` e `"pompa"`, la chiave e `"pompa"`.

---

## Command

Per inviare un comando al device, pubblica sul topic `guiver/{id}/command`:

```json
{"cmd": "set_pompa", "value": true}
```

| Campo | Tipo | Obbligatorio | Descrizione |
|---|---|---|---|
| `cmd` | string | si | Command name (vedi announce) |
| `value` | bool | no | Valore desiderato. Se omesso, il relay/LED viene toggled |

### Comandi disponibili

| Command name | Effetto | Parametri |
|---|---|---|
| `set_{RELAY_NAME}` | Accende/spegne il relay | `value` (bool, opzionale) |
| `set_led` | Accende/spegne il LED RGB | `value` (bool, opzionale) |

### Esempi

```bash
# Accendere il relay
mosquitto_pub -h raspberrypi.local -t "guiver/esp_control_base/command" \
  -m '{"cmd": "set_pompa", "value": true}'

# Spegnere il relay
mosquitto_pub -h raspberrypi.local -t "guiver/esp_control_base/command" \
  -m '{"cmd": "set_pompa", "value": false}'

# Toggle del relay (senza value)
mosquitto_pub -h raspberrypi.local -t "guiver/esp_control_base/command" \
  -m '{"cmd": "set_pompa"}'

# Accendere il LED
mosquitto_pub -h raspberrypi.local -t "guiver/esp_control_base/command" \
  -m '{"cmd": "set_led", "value": true}'
```

---

## Response

Dopo ogni comando, il device pubblica una risposta su `guiver/{id}/response`:

```json
{
  "status": "ok",
  "state": {
    "pompa": true
  }
}
```

| Campo | Tipo | Descrizione |
|---|---|---|
| `status` | string | `"ok"` o `"error"` |
| `state` | object | Stato aggiornato degli attuatori |

---

## Interazione con mosquitto

### Ascoltare tutti i topic del device

```bash
mosquitto_sub -h raspberrypi.local -t "guiver/esp_control_base/#" -v
```

### Esempio di sessione completa

```bash
# 1. Ascolta i topic
mosquitto_sub -h raspberrypi.local -t "guiver/esp_control_base/#" -v &

# 2. Accendi il relay
mosquitto_pub -h raspberrypi.local -t "guiver/esp_control_base/command" \
  -m '{"cmd": "set_pompa", "value": true}'

# Output atteso:
# guiver/esp_control_base/response → {"status":"ok","state":{"pompa":true}}
# guiver/esp_control_base/status → {"temperature":23.5,"humidity":45.2,"pompa":true,"led":false}

# 3. Spegni il relay
mosquitto_pub -h raspberrypi.local -t "guiver/esp_control_base/command" \
  -m '{"cmd": "set_pompa", "value": false}'

# 4. Toggle del LED
mosquitto_pub -h raspberrypi.local -t "guiver/esp_control_base/command" \
  -m '{"cmd": "set_led"}'
```
