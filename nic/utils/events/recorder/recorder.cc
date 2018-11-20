#include "nic/utils/events/recorder/recorder.hpp"
#include "nic/utils/events/recorder/constants.h"
#include "nic/utils/events/queue/queue.hpp"
#include "gen/proto/events.pb.h"
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

// singleton recorder object
events_recorder *recorder;

// initialize the events recorder; size is defaulted to SHM_SIZE if undefined (shm_size = 0)
int events_recorder::init(const char* shm_name, int shm_size, const char *component, const ::google::protobuf::EnumDescriptor* event_types_descriptor)
{
    if (!event_types_descriptor) {
        printf("event_types descriptor is required\n");
        return -1;
    }

    if (!recorder) {
        if(shm_size == 0) {
            shm_size = SHM_SIZE;
        }

        std::string abs_shm_name = shm_name; // /dev/shm/{shm_name}

        if (shm_name[0] != '/') {            // /dev/shm/pen-events/{shm_name}
            std::string dir_path = "/dev/shm" + std::string(SHM_DIR);

            // create /dev/shm/{SHM_DIR} if not exists
            DIR *dir;
            if ((dir = opendir(dir_path.c_str())) == nullptr && errno == ENOENT) { // not exists
                int ret_val = mkdir(SHM_DIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); //0775
                if (ret_val == -1) {
                    return ret_val;
                }
            }

            abs_shm_name =  std::string(SHM_DIR) + shm_name;
        }

        // create events queue
        events_queue *evts_queue = events_queue::init(abs_shm_name.c_str(), shm_size, SHM_BUF_SIZE);
        if (!evts_queue) {
            printf("failed to create events queue\n");
            return -1;
        }

        recorder = new(events_recorder);
        recorder->component_ = component;
        recorder->queue_ = evts_queue;
        recorder->event_types_descriptor = event_types_descriptor;
    }

    return 0;
}

// tear down the underlying events queue
void events_recorder::deinit()
{
    if (recorder) {
        recorder->queue_->deinit();
        recorder = nullptr;
    }
}

// construct and record event in the shared memory queue
int events_recorder::event(events::Severity severity, const int type, const char* kind, const ::google::protobuf::Message& key, const char* msg...)
{
    if (!recorder) {
        printf("recorder not created\n");
        return -1;
    }

    // this ensures the event type belongs to given descriptor
    const std::string event_type_str = ::google::protobuf::internal::NameOfEnum(recorder->event_types_descriptor, type);
    if (event_type_str.empty()) {
        printf("event type does not exist: %d\n", type);
        return -1;
    }

    events::Event evt;
    va_list args;
    char buffer[256];                           // message buffer

    evt.set_severity(severity);                 // severity
    evt.set_type(event_type_str.c_str());       // type
    evt.set_component(recorder->component_);    // component
    evt.set_time(std::time(0));                 // time
    evt.set_object_kind(kind);                  // object_kind
    evt.set_allocated_object_key(toAny(key));   // object_key

    va_start (args, msg);
    vsprintf (buffer, msg, args);
    va_end(args);
    evt.set_message(buffer);                    // message

    return write_event(evt);
}

// write given event to the shared memory queue
int events_recorder::write_event(events::Event evt)
{
    int size  = EVENT_SIZE(evt);

    uint8_t *buf = recorder->queue_->get_buffer(size);
    if(buf) {
        if (!evt.SerializeToArray(buf, size)) {
            return -1;
        }

        return recorder->queue_->write_msg_size(buf, size);
    }

    printf("requested buffer size (%d) not available\n", size);
    return -1;
}

// convert protobuf message to ::google::protobuf::Any*
::google::protobuf::Any *events_recorder::toAny(const ::google::protobuf::Message& key) {
    ::google::protobuf::Any *obj_key = ::google::protobuf::Any().New();
    obj_key->PackFrom(key);

    return obj_key;
}