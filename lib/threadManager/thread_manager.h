#include <vector>

struct Thread
{
    void (*function)();
    unsigned long interval = 150;
    unsigned long lastUpdate = 0;
};

class ThreadManager
{
    std::vector<Thread> thread_list;

public:
    void thread_loop();
    void add_method(void (*thread_func)(), unsigned long interval = 150);
};