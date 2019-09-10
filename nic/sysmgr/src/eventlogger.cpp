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
            "sysmgr", logger->get_spd());
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
