#ifndef __EVENTLOGGER_HPP__
#define __EVENTLOGGER_HPP__

#include <memory>

#include "gen/proto/sysmgr_events.pb.h"
#include "nic/utils/events/recorder/recorder.hpp"

class EventLogger {
    static std::shared_ptr<EventLogger> instance;
    events_recorder* recorder;
public:
    static std::shared_ptr<EventLogger> getInstance();
    void LogSystemEvent(sysmgr_events::Event event, std::string description);
    void LogServiceEvent(sysmgr_events::Event event, std::string description,
       std::string name);
};
typedef std::shared_ptr<EventLogger> EventLoggerPtr;

#endif // __EVENTLOGGER_HPP__
