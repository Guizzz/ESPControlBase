#include <Arduino.h>
#include <Wire.h>
#include <config.h>

#if ENABLE_DISPLAY
#include <display_manager.h>
#endif

#include <thread_manager.h>
#include <request_manager.h>
#include <mqtt_manager.h>

#if ENABLE_SENSOR_GY21
#include <GY_21.h>
#endif

#if ENABLE_LED
#include <ChainableLED.h>
#endif

// ── Display OLED ──────────────────────────────────────────────────
#if ENABLE_DISPLAY
DisplayManager display_manager(DISPLAY_TIMEOUT * 1000UL);
#endif

// ── Sensori ───────────────────────────────────────────────────────
#if ENABLE_SENSOR_GY21
GY21 sensor;
#endif

// ── Gestori ───────────────────────────────────────────────────────
ThreadManager threadManager;
WiFiServer server(80);
RequestManager request_manager(&server);
MqttManager mqtt_manager(
    DEVICE_ID, DEVICE_NAME, DEVICE_TYPE,
    WIFI_SSID, WIFI_PSW,
    MQTT_HOST, MQTT_PORT,
    STATUS_INTERVAL,
    MQTT_TOPIC_PREFIX
);

// ── LED RGB ───────────────────────────────────────────────────────
#if ENABLE_LED
ChainableLED leds(LED_CLOCK, LED_DATA, 1);
bool running = false;
float hue = 0.0;
float brightness = 0;
#define MAX_BRIGHT 0.5f
#endif

// ── Relay ─────────────────────────────────────────────────────────
#if ENABLE_RELAY
bool prev_relay = false;
bool relay = false;
#if RELAY_TYPE == RELAY_MOMENTARY
unsigned long relay_on_time = 0;
#endif
#endif

// ── Gestione LED ──────────────────────────────────────────────────
#if ENABLE_LED
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

    #if ENABLE_DISPLAY
    display_manager.show_led(running);
    #endif
}
#endif

// ── Gestione Relay ────────────────────────────────────────────────
#if ENABLE_RELAY
void manage_relay()
{
    #if RELAY_TYPE == RELAY_MOMENTARY
    if (relay && millis() - relay_on_time >= RELAY_PULSE_MS) {
        relay = false;
    }
    #endif

    if (prev_relay == relay)
        return;

    prev_relay = relay;
    digitalWrite(RELAY_PIN, relay ? HIGH : LOW);
    #if ENABLE_DISPLAY
    display_manager.show_relay(relay, RELAY_NAME);
    #endif
}
#endif

// ── Gestione Temperatura (display) ────────────────────────────────
#if ENABLE_SENSOR_GY21
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

    #if ENABLE_DISPLAY
    display_manager.show_temp(last_val_t, last_val_h);
    #endif
}
#endif

// ── Pulsante (debounce + fronte di discesa) ─────────────────────
void manage_button()
{
    static bool last_reading = HIGH;
    static bool last_stable = HIGH;
    static unsigned long last_change = 0;

    bool curr = digitalRead(FLASH_BUTTON);

    if (curr != last_reading)
    {
        last_change = millis();
        last_reading = curr;
    }

    if (millis() - last_change >= 50)
    {
        if (last_stable == HIGH && last_reading == LOW)
            #if ENABLE_DISPLAY
            display_manager.activity();
            #endif
        last_stable = last_reading;
    }
}

// ── Flush Display (thread periodico) ────────────────────────────
#if ENABLE_DISPLAY
void update_display()
{
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
#endif

// ── Costruttore payload status per MQTT ──────────────────────────
JsonDocument build_status()
{
    JsonDocument doc;

    #if ENABLE_SENSOR_GY21
    static unsigned long last_read = 0;
    static float last_temp = 0;
    static float last_hum = 0;
    unsigned long now = millis();

    if (now - last_read >= 3000) {
        last_temp = sensor.GY21_Temperature();
        last_hum  = sensor.GY21_Humidity();
        last_read = now;
    }

    doc["temperature"] = last_temp;
    doc["humidity"]    = last_hum;
    #endif

    #if ENABLE_RELAY
    doc[RELAY_NAME]       = relay;
    #endif

    #if ENABLE_LED
    doc["led"]         = running;
    #endif

    return doc;
}

// ── Handler comandi ───────────────────────────────────────────────

#if ENABLE_RELAY
JsonDocument set_relay(JsonDocument param)
{
    if (!param["value"].isNull())
        relay = param["value"].as<bool>();
    else
        relay = !relay;

    #if RELAY_TYPE == RELAY_MOMENTARY
    if (relay) relay_on_time = millis();
    #endif

    JsonDocument ret;
    ret[RELAY_NAME] = relay;
    return ret;
}
#endif

#if ENABLE_LED
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
#endif

#if ENABLE_SENSOR_GY21
JsonDocument get_temp(JsonDocument param)
{
    float temp = sensor.GY21_Temperature();
    float hum  = sensor.GY21_Humidity();
    JsonDocument ret;
    ret["temp"] = temp;
    ret["hum"]  = hum;
    return ret;
}
#endif

// ── Setup ─────────────────────────────────────────────────────────
void setup()
{
    Serial.begin(9600);
    Wire.begin(SDA, SCL);

    #if ENABLE_RELAY
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
    #endif

    pinMode(FLASH_BUTTON, INPUT_PULLUP);

    #if ENABLE_DISPLAY
    display_manager.begin();
    display_manager.show_message("Avvio ESPControlBase...");
    #endif

    // ── Registrazione handler MQTT ──
    #if ENABLE_DISPLAY
    display_manager.show_message("Registro handlers...");
    #endif
    #if ENABLE_RELAY
    mqtt_manager.on_command(RELAY_CMD, &set_relay, RELAY_NAME);
    #endif
    #if ENABLE_LED
    mqtt_manager.on_command(LED_CMD, &set_led, LED_NAME);
    #endif
    mqtt_manager.on_status(&build_status);
    
    // ── Registrazione route HTTP legacy ──
    #if ENABLE_SENSOR_GY21
    request_manager.add_request("GET",  "/get_temp",  &get_temp);
    #endif
    #if ENABLE_RELAY
    request_manager.add_request("POST", "/set_relay", &set_relay);
    #endif
    #if ENABLE_LED
    request_manager.add_request("POST", "/set_led",   &set_led);
    #endif
    
    // ── Thread periodici ──
    #if ENABLE_LED
    threadManager.add_method(&manage_led, 50);
    #endif
    #if ENABLE_RELAY
    threadManager.add_method(&manage_relay);
    #endif
    threadManager.add_method(&manage_button, 50);
    #if ENABLE_SENSOR_GY21
    threadManager.add_method(&manage_temp, 150);
    #endif
    #if ENABLE_DISPLAY
    threadManager.add_method(&update_display, 500);
    #endif
    
    // WiFi + MQTT (gestisce anche la connessione WiFi)
    #if ENABLE_DISPLAY
    display_manager.show_message("Connessione WiFi...");
    #endif
    mqtt_manager.begin();
    
    #if ENABLE_DISPLAY
    display_manager.show_message("Avvio completato!");

    display_manager.show_startup(
        WiFi.localIP().toString().c_str(),
        mqtt_manager.is_connected()
    );
    #if ENABLE_RELAY
    display_manager.show_relay(relay, RELAY_NAME);
    #endif
    #endif
    
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
