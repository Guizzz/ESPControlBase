#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <list>

#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

// ── Struttura per registrare un handler di comando ────────────────
struct CommandHandler
{
    String cmd;
    JsonDocument (*handler)(JsonDocument param);
};

class MqttManager
{
public:
    MqttManager(
        const char* device_id,
        const char* device_name,
        const char* device_type,
        const char* wifi_ssid,
        const char* wifi_psw,
        const char* mqtt_host,
        uint16_t mqtt_port,
        unsigned long status_interval = 60
    );

    // Avvia WiFi e connessione MQTT
    void begin();

    // loop: mantiene connessione MQTT + publish periodico
    void loop();

    // Registra una funzione da chiamare per costruire il payload status
    void on_status(JsonDocument (*status_builder)());

    // Registra un handler per un comando MQTT
    void on_command(const char* cmd, JsonDocument (*handler)(JsonDocument param));

    // Forza pubblicazione immediata dello status
    void publish_status();

    // Pubblica una risposta sul topic response
    void publish_response(const char* status, JsonDocument* state = nullptr);

    bool is_connected();
    const char* get_device_id() const;

private:
    // ── Configurazione ────────────────────────────────────────────
    String _device_id;
    String _device_name;
    String _device_type;
    String _wifi_ssid;
    String _wifi_psw;
    String _mqtt_host;
    uint16_t _mqtt_port;
    unsigned long _status_interval;

    // ── Client ────────────────────────────────────────────────────
    WiFiClient _wifi_client;
    PubSubClient _mqtt_client;

    // ── Callback ──────────────────────────────────────────────────
    JsonDocument (*_status_builder)() = nullptr;
    std::list<CommandHandler> _command_handlers;

    // ── Timing ────────────────────────────────────────────────────
    unsigned long _last_status = 0;
    unsigned long _last_reconnect_attempt = 0;

    // ── Flag ──────────────────────────────────────────────────────
    bool _first_connect = true;
    bool _subscribed = false;

    // ── Metodi privati ────────────────────────────────────────────
    void connect_wifi();
    void connect_mqtt();
    void publish_announce();
    void publish_online();

    // Callback MQTT statica (PubSubClient richiede funzione libera)
    static MqttManager* _instance;
    static void mqtt_callback_static(char* topic, byte* payload, unsigned int length);
    void on_mqtt_message(char* topic, byte* payload, unsigned int length);

    // Helper per pubblicare JSON
    void publish_json(const char* topic, JsonDocument& doc, bool retained);
};

#endif
