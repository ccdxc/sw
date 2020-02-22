//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains event identifiers, event data definitions and related
/// APIs
///
//----------------------------------------------------------------------------

#ifndef __CORE_EVENT_HPP__
#define __CORE_EVENT_HPP__

#include <signal.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/apollo/api/pds_state.hpp"

// event identifiers
typedef enum event_id_e {
    EVENT_ID_NONE               = 0,
    EVENT_ID_PDS_HAL_UP         = 1,
    EVENT_ID_PORT_STATUS        = 2,
    EVENT_ID_XCVR_STATUS        = 3,
    EVENT_ID_UPLINK_STATUS      = 4,
    EVENT_ID_HOST_LIF_CREATE    = 5,
    EVENT_ID_LIF_STATUS         = 6,
    EVENT_ID_MAC_LEARN          = 7,
    EVENT_ID_IP_LEARN           = 8,
    EVENT_ID_MAC_AGE            = 9,
    EVENT_ID_IP_AGE             = 10,
} event_id_t;

namespace core {

// port event specific information
typedef struct port_event_info_s {
    pds_ifindex_t    ifindex;
    port_event_t     event;
    port_speed_t     speed;
} port_event_info_t;

// xcvr event specific information
typedef struct xcvr_event_info_s {
    pds_ifindex_t    ifindex;
    xcvr_state_t     state;
    xcvr_pid_t       pid;
    cable_type_t     cable_type;
    uint8_t          sprom[XCVR_SPROM_SIZE];
} xcvr_event_info_t;

// uplink interface event specific information {
typedef struct uplink_event_info_s {
    pds_ifindex_t     ifindex;
    pds_if_state_t    state;
} uplink_event_info_t;

// lif event specific information
typedef struct lif_event_info_s {
    pds_ifindex_t    ifindex;
    char             name[SDK_MAX_NAME_LEN];
    mac_addr_t       mac;
    lif_state_t      state;
} lif_event_info_t;

// MAC, IP learn specific information
typedef struct learn_event_info_s {
    pds_obj_key_t   vpc;
    pds_obj_key_t   subnet;
    pds_ifindex_t   ifindex;
    ip_addr_t       ip_addr;
    mac_addr_t      mac_addr;
} learn_event_info_t;

// event structure that gets passed around for every event
typedef struct event_s {
    event_id_t              event_id;
    union {
        port_event_info_t   port;
        xcvr_event_info_t   xcvr;
        uplink_event_info_t uplink;
        lif_event_info_t    lif;
        learn_event_info_t  learn;
    };
} event_t;

///< \brief    allocate event memory
///< \return    allocated event instance
event_t *event_alloc(void);

///< \brief    free event memory
///< \param[in] event    event to be freed back
void event_free(event_t *event);

///< enqueue event to a given thread
///< event    event to be enqueued
///< \param[in] thread_id    id of the thread to enqueue the event to
///< \return    true if the operation succeeded or else false
bool event_enqueue(event_t *event, uint32_t thread_id);

///< \brief    dequeue event from given thread
///< \param[in] thread_id    id of the thread from which event needs
///<                         to be dequeued from
event_t *event_dequeue(uint32_t thread_id);

}    // namespace core

#endif    // __CORE_EVENT_HPP__
