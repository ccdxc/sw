#include <memory>
#include <string>

#include "bus_api.hpp"
#include "log.hpp"
#include "service_factory.hpp"
#include "service_watcher.hpp"
#include "switchroot_watcher.hpp"
#include "utils.hpp"

void
bus_up (std::string bus_name)
{
    g_log->debug("Service %s started", bus_name.c_str());
    ServiceLoop::getInstance()->queue_event(
        ServiceEvent::create(bus_name, SERVICE_EVENT_START));
    g_events->SystemBooted();
}

void
service_started (std::string process_name)
{
    g_log->debug("Service %s started", process_name.c_str());
    ServiceLoop::getInstance()->queue_event(
        ServiceEvent::create(process_name, SERVICE_EVENT_START));
}

void
service_heartbeat (std::string process_name)
{
    g_log->debug("Service %s heartbeat", process_name.c_str());
    ServiceLoop::getInstance()->queue_event(
        ServiceEvent::create(process_name, SERVICE_EVENT_HEARTBEAT));
}

void
switchroot (void)
{
    g_log->info("Switching root");
    switch_root();
    SwitchrootLoop::getInstance()->set_switchroot();
}

void
respawn_processes (void)
{
    g_log->info("Respawning processes");
    ServiceFactory::getInstance()->respawn_all();
}

bus_api_t g_bus_callbacks = {
    bus_up,
    service_started,
    service_heartbeat,
    switchroot,
    respawn_processes,
};
