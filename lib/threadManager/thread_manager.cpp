#include "thread_manager.h"
#include <Arduino.h>


void ThreadManager::thread_loop()
{
    unsigned long now = millis();

    if (now - lastUpdate < speed) 
        return;

    lastUpdate = now;

    for (auto t: thread_list)
        t.function();
}

void ThreadManager::add_method(void (*thread_func)())
{
    Thread new_t;
    new_t.function = thread_func;
    thread_list.push_back(new_t);
}
