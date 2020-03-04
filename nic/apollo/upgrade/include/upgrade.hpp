//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines upgrade enums
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_UPGRADE_HPP__
#define __INCLUDE_UPGRADE_HPP__

#include "nic/sdk/include/sdk/types.hpp"
#include "nic/apollo/include/globals.hpp"

/// \defgroup UPG Upgrade Manager
/// @{


/// \brief upgrade stages
typedef enum upg_stage_e {
    UPG_STAGE_NONE = 0,         ///< invalid
    UPG_STAGE_COMPAT_CHECK,     ///< upgarde ready check
    UPG_STAGE_START,            ///< start an upgrade
    UPG_STAGE_PREPARE,          ///< prepare for an upgrade
    UPG_STAGE_ABORT,            ///< abort the on-going upgrade
    UPG_STAGE_INIT,             ///< initialize the new upgrade
    UPG_STAGE_ROLLBACK,         ///< rollback to the previous version
    UPG_STAGE_SWITCHOVER,       ///< switch to the new version
    UPG_STAGE_EXIT,             ///< exit previous or new depends on upgrade status.
    UPG_STAGE_MAX,              ///< invalid
} upg_stage_t;

/// \brief upgrade modes
typedef enum upg_mode_e {
    UPG_MODE_NONE = 0,
    UPG_MODE_DISRUPTIVE,        ///< disruptive upgrade
    UPG_MODE_ISSU               ///< non disruptive in service software upgrade
} upg_mode_t;

/// \brief upgrade operational table state actions
typedef enum upg_oper_state_action_e {
    UPG_OPER_STATE_ACTION_NONE = 0,
    UPG_OPER_STATE_ACTION_COPY,       ///< copy operational state tables to new location in memory
                                      ///< with proper translation
    UPG_OPER_STATE_ACTION_FLUSH,      ///< flush the oprational state tables
    UPG_OPER_STATE_ACTION_PRESERVE,   ///< keep the operational state tables in the current location
                                      ///< with no changes.
    UPG_OPER_STATE_ACTION_REPLACE     ///< replace the existing operational state table
} upg_oper_state_action_t;

/// \brief upgrade responses
typedef enum upg_status_e {
    UPG_STATUS_OK = 0,     ///< operation successful
    UPG_STATUS_FAIL,       ///< operation failed, but system is stable
    UPG_STATUS_CRITICAL    ///< operation failed, and system is unstable
} upg_status_t;

/// \brief upgrade event msg
/// as the upgrade mgr need to communicate with old and new images,
/// we should not add anything in the middle.
/// TODO: should i convert to protobuf and send
typedef struct upg_event_msg_s {
    upg_stage_t    stage;                 ///< request stage
    upg_status_t   rsp_status;            ///< response status
    char           rsp_thread_name[64];   ///< response thread name
    uint32_t       rsp_thread_id;         ///< response thread id. can be used
                                          ///< to send unicast ipc to this thread
    // TODO other infos
} __PACK__ upg_event_msg_t;

// trace utilities
// stage to string
static const char *upg_stage_name[] =  {
    [UPG_STAGE_NONE]         = "none",
    [UPG_STAGE_COMPAT_CHECK] = "compatcheck",
    [UPG_STAGE_START]        = "start",
    [UPG_STAGE_PREPARE]      = "prepare",
    [UPG_STAGE_ABORT]        = "abort",
    [UPG_STAGE_INIT]         = "init",
    [UPG_STAGE_ROLLBACK]     = "rollback",
    [UPG_STAGE_SWITCHOVER]   = "switchover",
    [UPG_STAGE_EXIT]         = "exit",
};

static inline const char *
upg_stage2str (upg_stage_t stage)
{
    return upg_stage_name[stage];
}

// status to string
static const char *upg_status_name[] = {
    [UPG_STATUS_OK]         = "ok",
    [UPG_STATUS_FAIL]       = "fail",
    [UPG_STATUS_CRITICAL]   = "critical",
};

static inline const char *
upg_status2str (upg_status_t status)
{
    return upg_status_name[status];
}

/// @}

#endif    // __INCLUDE_UPGRADE_HPP__
