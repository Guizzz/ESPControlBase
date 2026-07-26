#ifndef UPDATE_FW_MANAGER_H
#define UPDATE_FW_MANAGER_H

#include <ArduinoOTA.h>

class UpdateFWManager {
public:
    UpdateFWManager();
    void begin(const char* hostname);
    void handle();

private:
    void setup_callbacks();
};

#endif
