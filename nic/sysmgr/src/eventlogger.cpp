#include "eventlogger.hpp"

#include <memory>

#include "gen/proto/events.pb.h"
#include "gen/proto/sysmgr_events.pb.h"
#include "nic/utils/events/recorder/recorder.hpp"

#include "utils.hpp"

EventLoggerPtr EventLogger::instance = nullptr;

EventLoggerPtr EventLogger::getInstance()
{
    if (instance == nullptr)
    {
	instance = std::make_shared<EventLogger>();

	instance->recorder = events_recorder::init(
	    "sysmgr.events", 32768, "sysmgr",
	    sysmgr_events::Event_descriptor(), logger->get_spd());
    }

    return instance;
}

void EventLogger::LogServiceEvent(sysmgr_events::Event event, std::string description,
    std::string name)
{
    sysmgr_events::EventKey key;

    key.set_name(name);

    logger->debug("Logging {} server event {}", name, description);
    this->recorder->event(events::INFO, event, "SysmgrProcessStatus", key,
	description.c_str(), name.c_str());
}


void EventLogger::LogSystemEvent(sysmgr_events::Event event,
    std::string description)
{
    sysmgr_events::EventKey key;

    key.set_name("default");

    logger->debug("Logging system event {}", description);
    this->recorder->event(events::INFO, event, "SysmgrSystemStatus", key,
	description.c_str());
}
