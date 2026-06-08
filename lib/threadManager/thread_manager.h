#include <list>

struct Thread
{
    void (*function)();
    unsigned long interval = 150;
    unsigned long lastUpdate = 0;
};

class ThreadManager
{
    std::list<Thread> thread_list;

public:
    void thread_loop();
    void add_method(void (*thread_func)(), unsigned long interval = 150);
};