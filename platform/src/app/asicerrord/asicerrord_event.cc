#include "asicerrord_event.hpp"

#include <memory>

#include "gen/proto/events.pb.h"
#include "nic/utils/events/recorder/recorder.hpp"
#include "gen/proto/eventtypes.pb.h"
#include "gen/proto/attributes.pb.h"

EventLoggerPtr EventLogger::instance = nullptr;

EventLoggerPtr EventLogger::getInstance()
{
    if (instance == nullptr) {
        instance = std::make_shared<EventLogger>();
        instance->recorder = events_recorder::init("asicerrord", GetLogger());
    }
    return instance;
}

void EventLogger::LogInterruptEvent(char *description)
{
    this->recorder->event(eventtypes::NAPLES_FATAL_INTERRUPT, description);
}
