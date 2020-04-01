//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines event definitions
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_EVENT_HPP__
#define __INCLUDE_API_PDS_EVENT_HPP__

#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/linkmgr/linkmgr.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/api/include/pds_lif.hpp"
#include "nic/apollo/api/include/pds_upgrade.hpp"

/// \brief events from PDS HAL
typedef enum pds_event_id_e {
    PDS_EVENT_ID_NONE,
    PDS_EVENT_ID_PORT_CREATE,
    PDS_EVENT_ID_PORT_UP,
    PDS_EVENT_ID_PORT_DOWN,
    PDS_EVENT_ID_LIF_CREATE,
    PDS_EVENT_ID_LIF_UPDATE,
    PDS_EVENT_ID_LIF_UP,
    PDS_EVENT_ID_LIF_DOWN,
    PDS_EVENT_ID_UPG,
    PDS_EVENT_ID_MAX,
} pds_event_id_t;

/// \brief host interface specific event information
typedef struct pds_lif_event_info_s {
    /// lif specification
    pds_lif_spec_t spec;
    /// lif operational status
    pds_lif_status_t status;
} pds_lif_event_info_t;

/// \brief physical port specific event information
typedef struct pds_port_info_s {
    /// unique identifier for the port
    pds_obj_key_t key;
    // TODO @akoradha port_args is exposed all the way to the agent
    //       with the current design, we should create port_spec_t,
    //       port_status_t and port_stats_t like any other object or
    //       better approach is to fold all the port stuff into if_entry
    //       and CLIs etc. will naturally work with current db walks etc.
    //       we have all eth ports in if db already.
    sdk::linkmgr::port_args_t info;
} pds_port_info_t;

/// \brief event information passed to event callback of the application
typedef struct pds_event_s {
    /// unique event id
    pds_event_id_t event_id;
    /// event specific information
    union {
        pds_port_info_t port_info;
        pds_lif_event_info_t lif_info;
        sdk::upg::upg_ev_params_t upg_params;
    };
} pds_event_t;

/// \brief type of the callback function invoked to raise events
typedef sdk_ret_t (*pds_event_cb_t)(const pds_event_t *event);

#endif    // __INCLUDE_API_PDS_EVENT_HPP__
