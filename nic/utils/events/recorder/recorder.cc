#include "nic/utils/events/recorder/recorder.hpp"
#include "nic/utils/events/recorder/constants.h"
#include "nic/utils/events/queue/queue.hpp"
#include "gen/proto/events.pb.h"
#include "nic/sdk/include/sdk/logger.hpp"
#include <google/protobuf/any.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <stdarg.h>
#include <ctime>
#include <sys/stat.h>
#include <dirent.h>
#include <error.h>

#define EVENT_SIZE(evt) evt.ByteSizeLong()

// initialize the events recorder; size is defaulted to SHM_SIZE if undefined (shm_size = 0)
events_recorder* events_recorder::init(const char* shm_name, int shm_size, const char *component, const ::google::protobuf::EnumDescriptor* event_types_descriptor)
{
    if (!event_types_descriptor) {
        SDK_TRACE_ERR("events_recorder {%s}: event_types enum descriptor is required", shm_name);
        return nullptr;
    }

    if(shm_size == 0) {
        shm_size = SHM_SIZE;
    }

    std::string abs_shm_name = shm_name;
    if (shm_name[0] != '/') {
        abs_shm_name =  std::string("/") + shm_name; // /dev/shm/{shm_name}
    }

    // create events queue
    events_queue *evts_queue = events_queue::init(abs_shm_name.c_str(), shm_size, SHM_BUF_SIZE);
    if (!evts_queue) {
        SDK_TRACE_ERR("events_recorder {%s}: failed to create events queue", shm_name);
        return nullptr;
    }

    events_recorder* recorder = new(events_recorder);
    recorder->component_ = component;
    recorder->queue_ = evts_queue;
    recorder->event_types_descriptor = event_types_descriptor;
    return recorder;
}

// tear down the underlying events queue
void events_recorder::deinit()
{
    this->queue_->deinit();
}

// construct and record event in the shared memory queue
int events_recorder::event(events::Severity severity, const int type, const char* kind, const ::google::protobuf::Message& key, const char* msg...)
{
    // this ensures the event type belongs to given descriptor
    const std::string event_type_str = ::google::protobuf::internal::NameOfEnum(this->event_types_descriptor, type);
    if (event_type_str.empty()) {
        SDK_TRACE_ERR("events_recorder {%s}: event type {%d} does not exist", this->queue_->get_name(), type);
        return -1;
    }

    events::Event evt;
    va_list args;
    char buffer[256];                           // message buffer

    evt.set_severity(severity);                 // severity
    evt.set_type(event_type_str.c_str());       // type
    evt.set_component(this->component_);        // component
    evt.set_time(std::time(0));                 // time
    evt.set_object_kind(kind);                  // object_kind
    evt.set_allocated_object_key(toAny(key));   // object_key

    va_start (args, msg);
    vsprintf (buffer, msg, args);
    va_end(args);
    evt.set_message(buffer);                    // message

    return this->write_event(evt);
}

// write given event to the shared memory queue
int events_recorder::write_event(events::Event evt)
{
    int size  = EVENT_SIZE(evt);

    uint8_t *buf = this->queue_->get_buffer(size);
    if(buf) {
        if (!evt.SerializeToArray(buf, size)) {
            return -1;
        }

        return this->queue_->write_msg_size(buf, size);
    }

    SDK_TRACE_ERR("events_recorder {%s}: requested buffer size {%d} not available", this->queue_->get_name(), size);
    return -1;
}

// convert protobuf message to ::google::protobuf::Any*
::google::protobuf::Any *events_recorder::toAny(const ::google::protobuf::Message& key) {
    ::google::protobuf::Any *obj_key = ::google::protobuf::Any().New();
    obj_key->PackFrom(key);

    return obj_key;
}