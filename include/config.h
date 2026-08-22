#ifndef CONFIG_H
#define CONFIG_H

// ── Identità dispositivo ──────────────────────────────────────────

#define DEVICE_ID       "salone_cacelletto"
#define DEVICE_NAME     "SaloneCancelletto"
#define DEVICE_TYPE     "temperature_gate"

// ── WiFi ──────────────────────────────────────────────────────────
#define WIFI_SSID       "casaGuizzz"
#define WIFI_PSW        "abcde12345"

// ── MQTT ──────────────────────────────────────────────────────────
#define MQTT_HOST       "192.168.1.25"   // broker MQTT: IP fisso evita la risoluzione mDNS (in affidabile su ESP8266) - era "bolide.local"
#define MQTT_PORT       1883
#define MQTT_TOPIC_PREFIX "guiver"           // prefisso topic MQTT

// ── Sensori ───────────────────────────────────────────────────────
#define STATUS_INTERVAL 5          // secondi tra publish status

// ── Display OLED ──────────────────────────────────────────────────
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
#define SCREEN_ADDRESS  0x3C

// ── Display ───────────────────────────────────────────────────────
#define DISPLAY_TIMEOUT 5          // secondi prima dello spegnimento auto

// ── Flag hardware ─────────────────────────────────────────────────
#define ENABLE_DISPLAY      1
#define ENABLE_SENSOR_GY21  1
#define ENABLE_LED          0
#define ENABLE_RELAY        1
#define ENABLE_STATUS_LED   1

// ── OTA ────────────────────────────────────────────────────────
#define OTA_HOSTNAME        "esp_control_base"
#define OTA_PASSWORD        "Lx6Wegf3kM2Z"     // password upload OTA (espota --auth)

// ── Relay ─────────────────────────────────────────────────────────
#define RELAY_NAME          "cancelletto"       // nome display/MQTT
#define RELAY_CMD           "set_" RELAY_NAME   // command name auto-derivato
#define RELAY_BISTABLE      0
#define RELAY_MOMENTARY     1
#define RELAY_TYPE          RELAY_MOMENTARY   // 0 = bistabile, 1 = momentary
#define RELAY_PULSE_MS      500              // durata impulso (solo MOMENTARY)

// ── Led ───────────────────────────────────────────────────────────
#define LED_NAME          "led_mensola"         // nome display/MQTT
#define LED_CMD           "set_" LED_NAME       // command name auto-derivato

// ── Status Led onboard ──────────────────────────────────────────
#define STATUS_LED_BLINK_MS 500              // durata blink ad ogni publish MQTT

// ── Pin ───────────────────────────────────────────────────────────
#define SDA             D5
#define SCL             D6

#define LED_DATA        D7
#define LED_CLOCK       D2

#define STATUS_LED_PIN  D4          // GPIO2 - led blu onboard modulo ESP-12F (attivo LOW)
#define RELAY_PIN       D1          // GPIO5
#define FLASH_BUTTON    D3          // GPIO0 - tasto flash integrato, LOW = premuto

#endif
