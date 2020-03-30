#include <memory>

#include <spdlog/spdlog.h>

#include "../events_api.hpp"

class DummyLogger : public SysmgrEvents {
    virtual void SystemBooted(void) override;
    virtual void ServiceStartedEvent(std::string name) override;
    virtual void ServiceStoppedEvent(std::string name) override;

};
typedef std::shared_ptr<DummyLogger> DummyLoggerPtr;

SysmgrEventsPtr
init_events (std::shared_ptr<spdlog::logger> logger)
{
    return std::make_shared<DummyLogger>();
}

void
DummyLogger::SystemBooted(void) {
}

void
DummyLogger::ServiceStartedEvent(std::string) {
}

void
DummyLogger::ServiceStoppedEvent(std::string) {
}
