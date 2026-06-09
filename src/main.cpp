#include <Arduino.h>
#include <Wire.h>

#include <display_manager.h>
#include <thread_manager.h>
#include <request_manager.h>
#include <mqtt_manager.h>
#include <GY_21.h>
#include <ChainableLED.h>
#include <config.h>

// ── Display OLED ──────────────────────────────────────────────────
DisplayManager display_manager(DISPLAY_TIMEOUT * 1000UL);

// ── Sensori ───────────────────────────────────────────────────────
GY21 sensor;

// ── Gestori ───────────────────────────────────────────────────────
ThreadManager threadManager;
WiFiServer server(80);
RequestManager request_manager(&server);
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

// ── Gestione LED ──────────────────────────────────────────────────
void shutdown()
{
    if (brightness <= 0.0)
        return;

    brightness -= 0.007;
    if (brightness < 0.0)
        brightness = 0.0;

    leds.setColorHSB(0, hue, 1.0, brightness);
}

void manage_led()
{
    if (!running)
    {
        shutdown();
    }
    else
    {
        if (brightness < MAX_BRIGHT) brightness += 0.007;
        if (brightness >= MAX_BRIGHT) brightness = MAX_BRIGHT;

        leds.setColorHSB(0, hue, 1.0, brightness);
        hue += 0.00067;

        if (hue >= 1.0)
            hue = 0.0;
    }

    display_manager.show_led(running);
}

// ── Gestione Relay ────────────────────────────────────────────────
void manage_relay()
{
    if (prev_relay == relay)
        return;

    prev_relay = relay;
    digitalWrite(RELAY_PIN, relay ? HIGH : LOW);
    display_manager.show_relay(relay);
}

// ── Gestione Temperatura (display) ────────────────────────────────
void manage_temp()
{
    static unsigned long last_read = 0;
    static float last_val_t = 0;
    static float last_val_h = 0;
    unsigned long now = millis();

    if (now - last_read >= 3000) {
        last_val_t = sensor.GY21_Temperature();
        last_val_h = sensor.GY21_Humidity();
        last_read = now;
    }

    display_manager.show_temp(last_val_t, last_val_h);
}

// ── Flush Display (thread periodico) ────────────────────────────
void update_display()
{
    if (digitalRead(FLASH_BUTTON) == LOW)
        display_manager.activity();

    static bool prev_mqtt = false;
    bool mqtt_ok = mqtt_manager.is_connected();
    if (mqtt_ok != prev_mqtt)
    {
        prev_mqtt = mqtt_ok;
        display_manager.show_status(
            WiFi.localIP().toString().c_str(),
            mqtt_ok
        );
    }
    display_manager.update();
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

    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
    pinMode(FLASH_BUTTON, INPUT_PULLUP);
    
    // ── Registrazione handler MQTT ──
    mqtt_manager.on_command("set_relay", &set_relay);
    mqtt_manager.on_command("set_led",   &set_led);
    mqtt_manager.on_status(&build_status);
    
    // ── Registrazione route HTTP legacy ──
    request_manager.add_request("GET",  "/get_temp",  &get_temp);
    request_manager.add_request("POST", "/set_relay", &set_relay);
    request_manager.add_request("POST", "/set_led",   &set_led);
    
    // ── Thread periodici ──
    threadManager.add_method(&manage_led, 50);
    threadManager.add_method(&manage_relay);
    threadManager.add_method(&manage_temp, 150);
    threadManager.add_method(&update_display, 500);
    
    // WiFi + MQTT (gestisce anche la connessione WiFi)
    mqtt_manager.begin();
    
    display_manager.begin();
    display_manager.show_startup(
        WiFi.localIP().toString().c_str(),
        mqtt_manager.is_connected()
    );
    display_manager.show_relay(relay);
    
    // Avvio server HTTP legacy
    server.begin();
}

// ── Loop ─────────────────────────────────────────────────────────
void loop()
{
    threadManager.thread_loop();
    request_manager.handle_request();
    mqtt_manager.loop();
}
