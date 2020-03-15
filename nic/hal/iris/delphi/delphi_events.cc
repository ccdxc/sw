//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "nic/include/base.hpp"
#include "nic/hal/hal_trace.hpp"
#include "nic/include/hal.hpp"
#include "nic/utils/events/recorder/recorder.hpp"
#include "gen/proto/eventtypes.pb.h"
#include "nic/hal/iris/delphi/delphi_events.hpp"

namespace hal {

static events_recorder   *g_hal_evt_recorder = NULL;

// Initializes event recorder
hal_ret_t hal_events_recorder_init()
{
    // initialize events recorder
    g_hal_evt_recorder= events_recorder::init("hal",
            std::shared_ptr<logger>(hal::utils::hal_logger()));

    if (g_hal_evt_recorder == nullptr) {
        HAL_TRACE_ERR("Events Recorder init failed");
        return HAL_RET_ERR;
    }
    HAL_TRACE_DEBUG("Initialized Events Recorder");

    return HAL_RET_OK;
}

// events_recorder_get returns the pointer to event recorder object
events_recorder*
hal_events_recorder_get(void)
{
    return g_hal_evt_recorder;
}

void hal_session_event_notify(eventtypes::EventTypes event_id)
{
    const char *descr = NULL;
    switch (event_id) {
    case (eventtypes::TCP_HALF_OPEN_SESSION_LIMIT_APPROACH):
        descr = "TCP half-open session limit approaching";
        break;
    case (eventtypes::TCP_HALF_OPEN_SESSION_LIMIT_REACHED):
        descr = "TCP half-open session limit reached";
        break;
    case (eventtypes::UDP_ACTIVE_SESSION_LIMIT_APPROACH):
        descr = "UDP active session limit approaching";
        break;
    case (eventtypes::UDP_ACTIVE_SESSION_LIMIT_REACHED):
        descr = "UDP active session limit reached";
        break;
    case (eventtypes::ICMP_ACTIVE_SESSION_LIMIT_APPROACH):
        descr = "ICMP active session limit approaching";
        break;
    case (eventtypes::ICMP_ACTIVE_SESSION_LIMIT_REACHED):
        descr = "ICMP active session limit reached";
        break;
    case (eventtypes::OTHER_ACTIVE_SESSION_LIMIT_APPROACH):
        descr = "UDP active session limit approaching";
        break;
    case (eventtypes::OTHER_ACTIVE_SESSION_LIMIT_REACHED):
        descr = "Other active session limit reached";
        break;
    default:
        HAL_TRACE_ERR("Unsupported HAL event");
        return;
    }

    if (hal_events_recorder_get()) {
        hal_events_recorder_get()->event(event_id, descr);
        HAL_TRACE_VERBOSE("Session limit event raised: {}", descr);
    }
    return;
}

}    // namespace hal

