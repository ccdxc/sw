//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/hal/hal_trace.hpp"
#include "nic/sdk/lib/catalog/catalog.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/utils/events/recorder/recorder.hpp"

namespace linkmgr {

static events_recorder *recorder;

sdk_ret_t
event_recorder_init (void) {
    // initialize events recorder
    recorder = events_recorder::init("linkmgr",
        std::shared_ptr<logger>(hal::utils::hal_logger()));

    if (recorder == nullptr) {
        HAL_TRACE_ERR("events recorder init failed");
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

// events_recorder_get returns the pointer to event recorder object
static events_recorder *
events_recorder_get(void)
{
    return recorder;
}

void
port_event_recorder_notify (port_event_info_t *port_event_info)
{
    const char *port_str;
    uint32_t logical_port = port_event_info->logical_port;
    port_event_t port_event = port_event_info->event;
    uint32_t ifindex =
                sdk::lib::catalog::logical_port_to_ifindex(logical_port);

    port_str = eth_ifindex_to_str(ifindex).c_str();

    switch (port_event) {
    case port_event_t::PORT_EVENT_LINK_UP:
        HAL_TRACE_DEBUG("port: {}, Link UP", port_str);
        events_recorder_get()->event(
                eventtypes::LINK_UP, "Port: %s, Link UP", port_str);
        break;
    case port_event_t::PORT_EVENT_LINK_DOWN:
        HAL_TRACE_DEBUG("port: {}, Link DOWN", port_str);
        events_recorder_get()->event(
                eventtypes::LINK_DOWN, "Port: %s, Link DOWN", port_str);
        break;
    default:
        break;
    }
}

}    // namespace linkmgr
