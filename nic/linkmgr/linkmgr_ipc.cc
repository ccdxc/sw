//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "linkmgr_src.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/hal/core/event_ipc.hpp"
#include "linkmgr_event_recorder.hpp"

namespace linkmgr {
namespace ipc {

void
port_event_notify (port_event_info_t *port_event_info)
{
    hal::core::event_t event;
    port_event_t port_event = port_event_info->event;
    port_speed_t port_speed = port_event_info->speed;
    port_fec_type_t fec_type = port_event_info->fec_type;
    uint32_t logical_port = port_event_info->logical_port;
    uint32_t ifindex = sdk::lib::catalog::logical_port_to_ifindex(logical_port);

    sdk::linkmgr::port_set_leds(logical_port, port_event);

    memset(&event, 0, sizeof(event));
    event.event_id = event_id_t::EVENT_ID_PORT_STATUS;
    event.port.id = ifindex;
    event.port.event = port_event;
    event.port.speed = port_speed;
    event.port.fec_type = fec_type;
    sdk::ipc::broadcast(event_id_t::EVENT_ID_PORT_STATUS, &event, sizeof(event));

    // publish to event recorder
    linkmgr::port_event_recorder_notify(port_event_info);
}

static void
send_xcvr_event (xcvr_event_info_t *xcvr_event_info)
{
    hal::core::event_t event;
    memset(&event, 0, sizeof(event));
    event.xcvr.id = xcvr_event_info->port_num;
    event.xcvr.state = xcvr_event_info->state;
    event.xcvr.pid = xcvr_event_info->pid;
    event.xcvr.cable_type = xcvr_event_info->cable_type;
    memcpy(event.xcvr.sprom, xcvr_event_info->xcvr_sprom, XCVR_SPROM_SIZE);
    sdk::ipc::broadcast(event_id_t::EVENT_ID_XCVR_STATUS, &event, sizeof(event));
}

void
xcvr_event_notify (xcvr_event_info_t *xcvr_event_info)
{
    uint32_t     port_num = xcvr_event_info->port_num;
    xcvr_state_t state    = xcvr_event_info->state;
    xcvr_pid_t   pid      = xcvr_event_info->pid;

    HAL_TRACE_DEBUG("Xcvr {}; port: {}, pid: {}", sdk::types::xcvrStateToStr(state), port_num, pid);
    send_xcvr_event(xcvr_event_info);
}
}    // namespace ipc
}    // namespace linkmgr
