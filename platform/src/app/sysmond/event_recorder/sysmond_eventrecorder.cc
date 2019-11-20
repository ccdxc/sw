#include <memory>
#include "gen/proto/events.pb.h"
#include "gen/proto/eventtypes.pb.h"
#include "gen/proto/attributes.pb.h"
#include "sysmond_eventrecorder.hpp"
#include "nic/utils/events/recorder/recorder.hpp"

EventLoggerPtr EventLogger::instance = nullptr;

EventLoggerPtr EventLogger::getInstance()
{
    if (instance == nullptr)
    {
        instance = std::make_shared<EventLogger>();

        instance->recorder = events_recorder::init(
            "sysmond", GetLogger());
    }
    return instance;
}

void EventLogger::LogCriticalTempEvent(std::string description)
{
    this->recorder->event(eventtypes::NAPLES_CATTRIP_INTERRUPT, description.c_str());
}

void EventLogger::LogOverTempAlarmEvent(std::string description, int temperature)
{
    TRACE_INFO(GetObflLogger(), "LogOverTempAlarmEvent::Over temp alarm.");
    this->recorder->event(eventtypes::NAPLES_OVER_TEMP, description.c_str(), temperature);
}

void EventLogger::LogOverTempExitAlarmEvent(std::string description, int temperature)
{
    this->recorder->event(eventtypes::NAPLES_OVER_TEMP_EXIT, description.c_str(), temperature);
}

void EventLogger::LogFatalInterruptEvent(const char *desc) {
    this->recorder->event(eventtypes::NAPLES_FATAL_INTERRUPT, desc);
}

void EventLogger::LogPanicEvent(std::string description)
{
    this->recorder->event(eventtypes::NAPLES_PANIC_EVENT, description.c_str());
}
