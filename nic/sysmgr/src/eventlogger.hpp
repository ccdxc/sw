#ifndef __EVENTLOGGER_HPP__
#define __EVENTLOGGER_HPP__

#include <memory>

#include "gen/proto/events.pb.h"
#include "nic/utils/events/recorder/recorder.hpp"
#include "gen/proto/eventtypes.pb.h"

class EventLogger {
    static std::shared_ptr<EventLogger> instance;
    events_recorder* recorder;
public:
    static std::shared_ptr<EventLogger> getInstance();
    void LogSystemEvent(eventtypes::EventTypes event, std::string description);
    void LogServiceEvent(eventtypes::EventTypes event, std::string description,
        std::string name);
};
typedef std::shared_ptr<EventLogger> EventLoggerPtr;

#endif // __EVENTLOGGER_HPP__
