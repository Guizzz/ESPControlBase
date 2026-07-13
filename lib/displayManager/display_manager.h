#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class DisplayManager
{
public:
    DisplayManager(unsigned long timeout_ms = 5000);

    void begin();
    void show_message(const char* msg);
    void show_startup(const char* ip, bool mqtt_connected);
    void show_status(const char* ip, bool mqtt_connected);
    void show_temp(float temperature, float humidity);
    void show_relay(bool state, const char* name = "relay");
    void show_led(bool state);
    void activity();
    void update();

private:
    Adafruit_SSD1306 _display;
    unsigned long _timeout;
    unsigned long _last_activity;
    bool _display_on;
};

#endif
