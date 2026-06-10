#ifndef CONFIG_H
#define CONFIG_H

// ── Identità dispositivo ──────────────────────────────────────────
#define DEVICE_ID       "temp_station"
#define DEVICE_NAME     "TempStation"
#define DEVICE_TYPE     "temperature"

// ── WiFi ──────────────────────────────────────────────────────────
#define WIFI_SSID       "CasaGuizzz-Camere"
#define WIFI_PSW        "abcde12345"

// ── MQTT ──────────────────────────────────────────────────────────
#define MQTT_HOST       "raspberrypi.local"
#define MQTT_PORT       1883

// ── Sensori ───────────────────────────────────────────────────────
#define STATUS_INTERVAL 5          // secondi tra publish status

// ── Display OLED ──────────────────────────────────────────────────
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
#define SCREEN_ADDRESS  0x3C

// ── Display ───────────────────────────────────────────────────────
#define DISPLAY_TIMEOUT 5          // secondi prima dello spegnimento auto

// ── Pin ───────────────────────────────────────────────────────────
#define SDA             D5
#define SCL             D6
#define LED_DATA        D7
#define LED_CLOCK       D2
#define RELAY_PIN       D1          // GPIO5
#define FLASH_BUTTON    D3          // GPIO0 - tasto flash integrato, LOW = premuto

#endif
