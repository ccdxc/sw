#ifndef __SYSYMOND_EVENTRECORDER_HPP__
#define __SYSYMOND_EVENTRECORDER_HPP__

#include <memory>
#include "gen/proto/events.pb.h"
#include "gen/proto/eventtypes.pb.h"
#include "platform/src/app/sysmond/logger.h"
#include "nic/utils/events/recorder/recorder.hpp"

class EventLogger {
    static std::shared_ptr<EventLogger> instance;
    events_recorder* recorder;
public:
    static std::shared_ptr<EventLogger> getInstance();
    void LogCriticalTempEvent(std::string description);
    void LogOverTempAlarmEvent(std::string description, int temperature);
    void LogOverTempExitAlarmEvent(std::string description, int temperature);
    void LogFatalInterruptEvent(const char *desc);
};
typedef std::shared_ptr<EventLogger> EventLoggerPtr;

#endif // __SYSYMOND_EVENTRECORDER_HPP__
