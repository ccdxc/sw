#ifndef __EVENTLOGGER_HPP__
#define __EVENTLOGGER_HPP__

#include <memory>

#include <spdlog/spdlog.h>

#include "gen/proto/events.pb.h"
#include "nic/utils/events/recorder/recorder.hpp"
#include "gen/proto/eventtypes.pb.h"

#include "../events_api.hpp"

class EventLogger : public SysmgrEvents {
    static std::shared_ptr<EventLogger> instance;
    events_recorder* recorder;
public:
    static std::shared_ptr<EventLogger> getInstance(
        std::shared_ptr<spdlog::logger> logger);
    virtual void SystemBooted(void) override;
    virtual void ServiceStartedEvent(std::string name) override;
    virtual void ServiceStoppedEvent(std::string name) override;
    void LogSystemEvent(eventtypes::EventTypes event,
                        std::string description);
    void LogServiceEvent(eventtypes::EventTypes event, std::string description,
                         std::string name);
};
typedef std::shared_ptr<EventLogger> EventLoggerPtr;

#endif // __EVENTLOGGER_HPP__
