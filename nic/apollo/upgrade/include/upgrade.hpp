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
#include "nic/sdk/include/sdk/platform.hpp"
#include "nic/apollo/include/globals.hpp"

/// \defgroup UPG Upgrade Manager
/// @{


/// \brief upgrade stages
typedef enum upg_stage_e {
    UPG_STAGE_NONE = 0,         ///< invalid
    UPG_STAGE_COMPAT_CHECK,     ///< upgarde ready check
    UPG_STAGE_START,            ///< start an upgrade
    UPG_STAGE_PREPARE,          ///< prepare for an upgrade
    UPG_STAGE_BACKUP,           ///< backup config for an upgrade
    UPG_STAGE_REPEAL,           ///< abort the on-going upgrade
    UPG_STAGE_ROLLBACK,         ///< rollback to the previous version
    UPG_STAGE_SYNC,             ///< config replay and opearational state synce
    UPG_STAGE_PREP_SWITCHOVER,  ///< quiescing gefore switch to the new version
    UPG_STAGE_SWITCHOVER,       ///< switch to the new version
    UPG_STAGE_READY,            ///< readiness check of new version
    UPG_STAGE_RESPAWN,          ///< restart new version
    UPG_STAGE_FINISH,           ///< failure or failure-non-recovarable status to netagent
    UPG_STAGE_EXIT,             ///< exit from upgrade.
    UPG_STAGE_MAX               ///< invalid
} upg_stage_t;

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
    upg_stage_t  stage;                   ///< request stage
    sdk::platform::upg_mode_t mode;       ///< upgrade mode
    upg_status_t   rsp_status;            ///< response status
    char           rsp_thread_name[64];   ///< response thread name
    uint32_t       rsp_thread_id;         ///< response thread id. can be used
                                          ///< to send unicast ipc to this thread
    void           *rsp_cookie;
    // TODO other infos
} __PACK__ upg_event_msg_t;

// trace utilities
// stage to string
static const char *upg_stage_name[] =  {
    [UPG_STAGE_NONE]            = "none",
    [UPG_STAGE_COMPAT_CHECK]    = "compatcheck",
    [UPG_STAGE_START]           = "start",
    [UPG_STAGE_PREPARE]         = "prepare",
    [UPG_STAGE_BACKUP]          = "backup",
    [UPG_STAGE_REPEAL]          = "repeal",
    [UPG_STAGE_ROLLBACK]        = "rollback",
    [UPG_STAGE_SYNC]            = "sync",
    [UPG_STAGE_PREP_SWITCHOVER] = "prepare_switchover",
    [UPG_STAGE_SWITCHOVER]      = "switchover",
    [UPG_STAGE_READY]           = "ready",
    [UPG_STAGE_RESPAWN]         = "respawn",
    [UPG_STAGE_FINISH]          = "finish",
    [UPG_STAGE_EXIT]            = "exit",
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
