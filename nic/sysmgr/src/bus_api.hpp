#ifndef __BUS_API_H__
#define __BUS_API_H__

#include <memory>
#include <string>

typedef struct bus_api_ {
    void (*bus_up)(std::string bus_name);
    void (*service_started)(std::string process_name);
    void (*service_heartbeat)(std::string process_name);
    void (*switchroot)(void);
    void (*respawn_processes)(void);
} bus_api_t;

class SysmgrBus {
public:
    virtual void Connect(void) = 0;
    virtual void SystemFault(std::string reason) = 0;
    virtual void ProcessDied(std::string name, pid_t pid,
                             std::string reason) = 0;
};
typedef std::shared_ptr<SysmgrBus> SysmgrBusPtr;

extern SysmgrBusPtr init_bus(bus_api_t *api);

#endif // __BUS_API_H__
