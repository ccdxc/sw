//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#ifndef __HAL_CORE_EVENT_HPP__
#define __HAL_CORE_EVENT_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"

namespace hal {
namespace core {

// event identifiers
typedef enum event_id_e {
    EVENT_ID_NONE               = 0,
    EVENT_ID_HAL_UP             = 1,
    EVENT_ID_PORT_STATUS        = 2,
    EVENT_ID_XCVR_STATUS        = 3,
    EVENT_ID_UPLINK_STATUS      = 4,
    EVENT_ID_HOST_LIF_CREATE    = 5,
    EVENT_ID_LIF_STATUS         = 6,
    EVENT_ID_UPG                = 7,
    EVENT_ID_UPG_STAGE_STATUS   = 8,
    EVENT_ID_MICRO_SEG          = 9,
    EVENT_ID_NCSI               = 10,
} event_id_t;

// port event specific information
typedef struct port_event_info_s {
    uint32_t         id;
    port_event_t     event;
    port_speed_t     speed;
} port_event_info_t;

// xcvr event specific information
typedef struct xcvr_event_info_s {
    uint32_t         id;
    xcvr_state_t     state;
    xcvr_pid_t       pid;
    cable_type_t     cable_type;
    uint8_t          sprom[XCVR_SPROM_SIZE];
} xcvr_event_info_t;

// micro segment event handle
typedef struct micro_seg_info_s {
    bool status;
} micro_seg_info_t;

// event structure that gets passed around for every event
typedef struct event_s {
    event_id_t              event_id;
    union {
        port_event_info_t   port;
        xcvr_event_info_t   xcvr;
        micro_seg_info_t    mseg;
    };
} event_t;

}   // namespace core
}   // namespace hal

using hal::core::event_id_t;

#endif   // __HAL_CORE_EVENT_HPP__

