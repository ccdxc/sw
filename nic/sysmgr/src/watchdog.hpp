#include <ctime>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

const int TIMEOUT = 15;

struct timeout {
    std::string name;
    time_t expires;
};

class Watchdog {
    public:
    Watchdog();
    void refresh(std::string name);
    int next_tick();
    std::list<std::string> expired();
    void dump();
    private:
    std::list<std::shared_ptr<struct timeout>> timeouts;
};