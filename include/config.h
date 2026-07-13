#ifndef CONFIG_H
#define CONFIG_H

// ── Identità dispositivo ──────────────────────────────────────────
#define DEVICE_ID       "esp_control_base"
#define DEVICE_NAME     "ESPControlBase"
#define DEVICE_TYPE     "generic"

// ── WiFi ──────────────────────────────────────────────────────────
#define WIFI_SSID       "CasaGuizzz-Camere"
#define WIFI_PSW        "abcde12345"

// ── MQTT ──────────────────────────────────────────────────────────
#define MQTT_HOST       "bolide.local"
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
#define ENABLE_LED          1
#define ENABLE_RELAY        1

// ── Relay ─────────────────────────────────────────────────────────
#define RELAY_NAME          "pompa"             // nome display/MQTT
#define RELAY_CMD           "set_" RELAY_NAME   // command name auto-derivato
#define RELAY_BISTABLE      0
#define RELAY_MOMENTARY     1
#define RELAY_TYPE          RELAY_BISTABLE   // 0 = bistabile, 1 = momentary
#define RELAY_PULSE_MS      500              // durata impulso (solo MOMENTARY)

// ── Led ───────────────────────────────────────────────────────────
#define LED_NAME          "led_mensola"         // nome display/MQTT
#define LED_CMD           "set_" LED_NAME       // command name auto-derivato

// ── Pin ───────────────────────────────────────────────────────────
#define SDA             D5
#define SCL             D6
#define LED_DATA        D7
#define LED_CLOCK       D2
#define RELAY_PIN       D1          // GPIO5
#define FLASH_BUTTON    D3          // GPIO0 - tasto flash integrato, LOW = premuto

#endif
