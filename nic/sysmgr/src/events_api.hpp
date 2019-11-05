#ifndef __SYSMGR_EVENTS_API_H__
#define __SYSMGR_EVENTS_API_H__

#include <memory>

#include <spdlog/spdlog.h>

class SysmgrEvents {
public:
    virtual void SystemBooted(void) = 0;
    virtual void ServiceStartedEvent(std::string name) = 0;
    virtual void ServiceStoppedEvent(std::string name) = 0;
};
typedef std::shared_ptr<SysmgrEvents> SysmgrEventsPtr;

extern SysmgrEventsPtr init_events(std::shared_ptr<spdlog::logger> logger);

#endif // __SYSMGR_EVENTS_API_H__
