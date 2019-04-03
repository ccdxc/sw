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
	    "sysmgr.events", 32768, 256, "sysmgr",
	    sysmgr_events::Event_descriptor(), logger->get_spd());
    }

    return instance;
}

void EventLogger::LogServiceEvent(std::string name, sysmgr_events::Event event,
    std::string description)
{
    sysmgr_events::EventKey key;

    key.set_name(name);
    
    this->recorder->event(events::INFO, event, "SysmgrProcessStatus", key,
	description.c_str());
}


void EventLogger::LogSystemEvent(sysmgr_events::Event event,
    std::string description)
{
    sysmgr_events::EventKey key;

    key.set_name("default");
    
    this->recorder->event(events::INFO, event, "SysmgrSystemStatus", key,
	description.c_str());
}
