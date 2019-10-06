#ifndef __EVENTLOGGER_HPP__
#define __EVENTLOGGER_HPP__

#include <memory>
#include "logger.h"
#include "gen/proto/events.pb.h"
#include "nic/utils/events/recorder/recorder.hpp"
#include "gen/proto/eventtypes.pb.h"

class EventLogger {
    static std::shared_ptr<EventLogger> instance;
    events_recorder* recorder;
public:
    static std::shared_ptr<EventLogger> getInstance();
    void LogFatalInterruptEvent(char *description);
};
typedef std::shared_ptr<EventLogger> EventLoggerPtr;

#endif // __EVENTLOGGER_HPP__
