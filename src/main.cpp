#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <thread_manager.h>
#include <request_manager.h>
#include <mqtt_manager.h>
#include <GY_21.h>
#include <ChainableLED.h>
#include <config.h>

// ── Display OLED ──────────────────────────────────────────────────
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ── Sensori ───────────────────────────────────────────────────────
GY21 sensor;

// ── Gestori ───────────────────────────────────────────────────────
ThreadManager threadManager;
WiFiServer server(80);
RequestManager request_manager(WIFI_SSID, WIFI_PSW, &server);
MqttManager mqtt_manager(
    DEVICE_ID, DEVICE_NAME, DEVICE_TYPE,
    WIFI_SSID, WIFI_PSW,
    MQTT_HOST, MQTT_PORT,
    STATUS_INTERVAL
);

// ── LED RGB ───────────────────────────────────────────────────────
ChainableLED leds(LED_CLOCK, LED_DATA, 1);
bool running = false;
float hue = 0.0;
float brightness = 0;
#define MAX_BRIGHT 0.5f

// ── Relay ─────────────────────────────────────────────────────────
bool prev_relay = false;
bool relay = false;

// ── OLED ──────────────────────────────────────────────────────────
void init_oled()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        return;
    }

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    display.println("OLED READY");
    display.println(WiFi.localIP().toString());
    display.display();
}

// ── Gestione LED ──────────────────────────────────────────────────
void shutdown()
{
    if (brightness <= 0.0)
        return;

    brightness -= 0.02;
    if (brightness < 0.0)
        brightness = 0.0;

    leds.setColorHSB(0, hue, 1.0, brightness);
}

void manage_led()
{
    if (!running)
    {
        shutdown();
        return;
    }

    if (brightness < MAX_BRIGHT) brightness += 0.02;
    if (brightness >= MAX_BRIGHT) brightness = MAX_BRIGHT;

    leds.setColorHSB(0, hue, 1.0, brightness);
    hue += 0.002;

    if (hue >= 1.0)
        hue = 0.0;
}

// ── Gestione Relay ────────────────────────────────────────────────
void manage_relay()
{
    if (prev_relay == relay)
        return;

    prev_relay = relay;

    display.setCursor(0, 48);
    display.fillRect(0, 48, 128, 16, SSD1306_BLACK);
    display.print(relay ? "ON " : "OFF");
    display.display();
}

// ── Gestione Temperatura (display) ────────────────────────────────
void manage_temp()
{
    static unsigned long last_read = 0;
    static float last_val = 0;
    unsigned long now = millis();

    if (now - last_read >= 3000) {
        last_val = sensor.GY21_Temperature();
        last_read = now;
    }

    display.setCursor(64, 48);
    display.fillRect(64, 48, 128, 16, SSD1306_BLACK);
    display.print(last_val);
    display.display();
}

// ── Costruttore payload status per MQTT ──────────────────────────
JsonDocument build_status()
{
    static unsigned long last_read = 0;
    static float last_temp = 0;
    static float last_hum = 0;
    unsigned long now = millis();

    if (now - last_read >= 3000) {
        last_temp = sensor.GY21_Temperature();
        last_hum  = sensor.GY21_Humidity();
        last_read = now;
    }

    JsonDocument doc;
    doc["temperature"] = last_temp;
    doc["humidity"]    = last_hum;
    doc["relay"]       = relay;
    doc["led"]         = running;
    return doc;
}

// ── Handler comandi ───────────────────────────────────────────────

JsonDocument set_relay(JsonDocument param)
{
    if (!param["value"].isNull())
        relay = param["value"].as<bool>();
    else
        relay = !relay;

    JsonDocument ret;
    ret["relay"] = relay;
    return ret;
}

JsonDocument set_led(JsonDocument param)
{
    if (!param["value"].isNull())
        running = param["value"].as<bool>();
    else
        running = !running;

    JsonDocument ret;
    ret["led"] = running;
    return ret;
}

JsonDocument get_temp(JsonDocument param)
{
    float temp = sensor.GY21_Temperature();
    float hum  = sensor.GY21_Humidity();
    JsonDocument ret;
    ret["temp"] = temp;
    ret["hum"]  = hum;
    return ret;
}

// ── Setup ─────────────────────────────────────────────────────────
void setup()
{
    Serial.begin(9600);
    Wire.begin(SDA, SCL);

    // WiFi + MQTT (gestisce anche la connessione WiFi)
    mqtt_manager.begin();

    init_oled();

    // Avvio server HTTP legacy
    server.begin();

    // Inizializzazione SPIFFS
    if (!SPIFFS.begin())
        Serial.println("Errore montaggio SPIFFS");

    // ── Registrazione handler MQTT ──
    mqtt_manager.on_command("set_relay", &set_relay);
    mqtt_manager.on_command("set_led",   &set_led);
    mqtt_manager.on_status(&build_status);

    // ── Registrazione route HTTP legacy ──
    request_manager.add_request("GET",  "/get_temp",  &get_temp);
    request_manager.add_request("POST", "/set_relay", &set_relay);
    request_manager.add_request("POST", "/set_led",   &set_led);

    // ── Thread periodici ──
    threadManager.add_method(&manage_led);
    threadManager.add_method(&manage_relay);
    threadManager.add_method(&manage_temp);
}

// ── Loop ─────────────────────────────────────────────────────────
void loop()
{
    threadManager.thread_loop();
    request_manager.handle_request();
    mqtt_manager.loop();
}
