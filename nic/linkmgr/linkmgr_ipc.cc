// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#include "linkmgr_src.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/hal/core/event_ipc.hpp"

namespace linkmgr {
namespace ipc {

void
port_event_notify (port_event_info_t *port_event_info)
{
    hal::core::event_t event;
    port_event_t port_event = port_event_info->event;
    port_speed_t port_speed = port_event_info->speed;
    uint32_t logical_port = port_event_info->logical_port;

    sdk::linkmgr::port_set_leds(logical_port, port_event);

    memset(&event, 0, sizeof(event));
    event.event_id = event_id_t::EVENT_ID_PORT_STATUS;
    event.port.id = logical_port;
    event.port.event = port_event;
    event.port.speed = port_speed;
    sdk::ipc::broadcast(event_id_t::EVENT_ID_PORT_STATUS, &event, sizeof(event));
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

    switch (state) {
    case xcvr_state_t::XCVR_REMOVED:
        HAL_TRACE_DEBUG("Xcvr removed; port: {}", port_num);

        send_xcvr_event(xcvr_event_info);
        break;

    case xcvr_state_t::XCVR_INSERTED:
        HAL_TRACE_DEBUG("Xcvr inserted; port: {}", port_num);

        send_xcvr_event(xcvr_event_info);
        break;

    case xcvr_state_t::XCVR_SPROM_READ:
        HAL_TRACE_DEBUG("Xcvr sprom read; port: {}, pid: {}", port_num, pid);

        send_xcvr_event(xcvr_event_info);
        break;

    case xcvr_state_t::XCVR_SPROM_READ_ERR:
        HAL_TRACE_DEBUG("Xcvr sprom read error; port: {}", port_num);

        send_xcvr_event(xcvr_event_info);
        break;

    default:
        break;
    }
}
}    // namespace ipc
}    // namespace linkmgr
