#include <list>

struct Thread
{
    void (*function)();
};

class ThreadManager
{

    std::list<Thread> thread_list;
    unsigned long lastUpdate = 0;
    unsigned long speed = 150;

public:
    void thread_loop();

    void add_method(void (*thread_func)());
};