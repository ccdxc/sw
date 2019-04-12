#ifndef __EVENTS_RECORDER_HPP__
#define __EVENTS_RECORDER_HPP__

#include "nic/utils/events/queue/queue.hpp"
#include "gen/proto/events.pb.h"
#include <google/protobuf/any.h>
#include <google/protobuf/message.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <stdint.h>
#include <spdlog/spdlog.h>

typedef std::shared_ptr<spdlog::logger> Logger;

class events_recorder {
public:
    // initialize events recorder; shm_name, shm_size, component, event types descriptor, logger
    static events_recorder* init(const char *, int, const char *, const ::google::protobuf::EnumDescriptor*, Logger);

    // tear down events recorder
    void deinit();

    // record event using the given args; // severity, type, kind, object key, message...
    int event(events::Severity, const int, const char*, const ::google::protobuf::Message&, const char*...);
private:
    const char *component_;
    events_queue *queue_;
    const ::google::protobuf::EnumDescriptor *event_types_descriptor;
    Logger logger_;

    // write given event to the underlying events queue
    int write_event(events::Event);

    // convert given protobuf message to any
    static ::google::protobuf::Any *toAny(const ::google::protobuf::Message&);
};

#endif
