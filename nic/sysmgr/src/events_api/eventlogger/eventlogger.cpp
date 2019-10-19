#include "eventlogger.hpp"

#include <memory>

#include <spdlog/spdlog.h>

#include "gen/proto/events.pb.h"
#include "nic/utils/events/recorder/recorder.hpp"
#include "gen/proto/eventtypes.pb.h"
#include "gen/proto/attributes.pb.h"


EventLoggerPtr EventLogger::instance = nullptr;

EventLoggerPtr EventLogger::getInstance(std::shared_ptr<spdlog::logger> logger)
{
    if (instance == nullptr)
    {
        instance = std::make_shared<EventLogger>();

        instance->recorder = events_recorder::init(
            "sysmgr", logger);
    }

    return instance;
}

void EventLogger::LogServiceEvent(eventtypes::EventTypes event,
    std::string description, std::string name)
{
    this->recorder->event(event, description.c_str(), name.c_str());
}


void EventLogger::LogSystemEvent(eventtypes::EventTypes event,
    std::string description)
{
    this->recorder->event(event, description.c_str());
}

void EventLogger::SystemBooted(void) {
    this->LogSystemEvent(eventtypes::SYSTEM_COLDBOOT, "System booted");
}

void EventLogger::ServiceStartedEvent(std::string name) {
    this->LogServiceEvent(eventtypes::SERVICE_STARTED,
                          "Service %s started", name);
}

void EventLogger::ServiceStoppedEvent(std::string name) {
    this->LogServiceEvent(eventtypes::NAPLES_SERVICE_STOPPED,
                          "Service %s stopped", name);
}

SysmgrEventsPtr
init_events (std::shared_ptr<spdlog::logger> logger)
{
    return EventLogger::getInstance(logger);
}
