#include <memory>

#include <spdlog/spdlog.h>

#include "../events_api.hpp"
#include "nic/operd/alerts/alerts.hpp"

class DummyLogger : public SysmgrEvents {
public:
    void SystemBooted(void) override;
    void ServiceStartedEvent(std::string name) override;
    void ServiceStoppedEvent(std::string name) override;
    DummyLogger();
private:
    operd::alerts::alert_recorder_ptr g_alerts;

};
typedef std::shared_ptr<DummyLogger> DummyLoggerPtr;

SysmgrEventsPtr
init_events (std::shared_ptr<spdlog::logger> logger)
{
    return std::make_shared<DummyLogger>();
}

DummyLogger::DummyLogger() {
    g_alerts = operd::alerts::alert_recorder::get();
}

void
DummyLogger::SystemBooted(void) {
    printf("ooops\n");
    fflush(stdout);
    g_alerts->alert(operd::alerts::SYSTEM_COLDBOOT, "System booted");
}

void
DummyLogger::ServiceStartedEvent(std::string svc) {
    printf("ooops\n");
    fflush(stdout);
    g_alerts->alert(operd::alerts::SERVICE_STARTED, "Service: %s", svc.c_str());
}

void
DummyLogger::ServiceStoppedEvent(std::string svc) {
    g_alerts->alert(operd::alerts::NAPLES_SERVICE_STOPPED, "Service: %s",
                    svc.c_str());
}
