#include "display_manager.h"

DisplayManager::DisplayManager(unsigned long timeout_ms)
    : _display(128, 64, &Wire, -1),
      _timeout(timeout_ms),
      _last_activity(0),
      _display_on(true)
{
}

void DisplayManager::begin()
{
    if (!_display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("SSD1306 allocation failed"));
        return;
    }

    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    _display.clearDisplay();
    _display.display();
}

void DisplayManager::show_message(const char* msg)
{
    _display.clearDisplay();
    _display.setCursor(0, 0);
    _display.println(msg);
    _display.display();
}

void DisplayManager::show_startup(const char* ip, bool mqtt_connected)
{
    _display.clearDisplay();
    _display.setCursor(0, 0);
    _display.println(ip);
    _display.println(mqtt_connected ? F("MQTT Connected") : F("MQTT Connection Error"));
    _display.display();
}

void DisplayManager::show_status(const char* ip, bool mqtt_connected)
{
    _display.fillRect(0, 0, 128, 16, SSD1306_BLACK);
    _display.setCursor(0, 0);
    _display.println(ip);
    _display.println(mqtt_connected ? F("MQTT Connected") : F("MQTT Connection Error"));
}

void DisplayManager::show_temp(float temperature, float humidity)
{
    _display.fillRect(0, 24, 128, 16, SSD1306_BLACK);
    _display.setCursor(0, 24);
    _display.print(F("Temp: "));
    _display.print(temperature);
    _display.println(F("C"));
    _display.setCursor(0, 32);
    _display.print(F("Hum:  "));
    _display.print(humidity);
    _display.println(F("%"));
}

void DisplayManager::show_relay(bool state)
{
    _display.fillRect(0, 48, 58, 8, SSD1306_BLACK);
    _display.setCursor(0, 48);
    _display.print(F("Relay: "));
    _display.print(state ? F("ON") : F("OFF"));
}

void DisplayManager::show_led(bool state)
{
    _display.fillRect(64, 48, 64, 8, SSD1306_BLACK);
    _display.setCursor(64, 48);
    _display.print(F("LED: "));
    _display.print(state ? F("ON") : F("OFF"));
}

void DisplayManager::activity()
{
    _last_activity = millis();
    if (!_display_on)
    {
        _display_on = true;
        _display.ssd1306_command(0xAF);
        _display.display();
    }
}

void DisplayManager::update()
{
    if (!_display_on)
        return;

    if (_timeout > 0 && millis() - _last_activity >= _timeout)
    {
        _display_on = false;
        _display.ssd1306_command(0xAE);
        return;
    }

    _display.display();
}
