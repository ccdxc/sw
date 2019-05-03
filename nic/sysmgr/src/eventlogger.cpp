#include "eventlogger.hpp"

#include <memory>

#include "gen/proto/events.pb.h"
#include "nic/utils/events/recorder/recorder.hpp"
#include "gen/proto/eventtypes.pb.h"
#include "gen/proto/attributes.pb.h"

#include "utils.hpp"

EventLoggerPtr EventLogger::instance = nullptr;

EventLoggerPtr EventLogger::getInstance()
{
    if (instance == nullptr)
    {
        instance = std::make_shared<EventLogger>();

        instance->recorder = events_recorder::init(
            "sysmgr.events", 32768, "sysmgr", logger->get_spd());
    }

    return instance;
}

void EventLogger::LogServiceEvent(eventtypes::EventTypes event,
    std::string description, std::string name)
{
    eventattrs::Key key;

    key.set_name(name);

    logger->debug("Logging {} server event {}", name, description);
    this->recorder->event(event, "SysmgrProcessStatus", key,
        description.c_str(), name.c_str());
}


void EventLogger::LogSystemEvent(eventtypes::EventTypes event,
    std::string description)
{
    eventattrs::Key key;

    key.set_name("default");

    logger->debug("Logging system event {}", description);
    this->recorder->event(event, "SysmgrSystemStatus", key,
        description.c_str());
}
