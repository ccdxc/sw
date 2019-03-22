//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains event identifiers and event data definitions
///
//----------------------------------------------------------------------------

#ifndef __CORE_EVENT_HPP__
#define __CORE_EVENT_HPP__

#include <signal.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace core {

// event identifiers
typedef enum event_id_e {
    EVENT_ID_NONE = 0,
    EVENT_ID_PORT = 1,
} event_id_t;

// port event specific information
typedef struct port_event_info_s {
    uint32_t        port_id;
    port_event_t    event;
    port_speed_t    speed;
} port_event_info_t;

// event structure that gets passed around for every event
typedef struct event_info_s {
    event_id_t               event_id;
    union {
        port_event_info_t    port;
    };
} event_info_t;

}    // namespace core

#endif    // __CORE_EVENT_HPP__
