#include "thread_manager.h"
#include <Arduino.h>


void ThreadManager::thread_loop()
{
    unsigned long now = millis();

    for (auto& t: thread_list)
    {
        if (now - t.lastUpdate >= t.interval)
        {
            t.lastUpdate = now;
            t.function();
        }
    }
}

void ThreadManager::add_method(void (*thread_func)(), unsigned long interval)
{
    Thread new_t;
    new_t.function = thread_func;
    new_t.interval = interval;
    thread_list.push_back(new_t);
}
