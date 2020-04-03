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

#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"
#include "include/sdk/platform.hpp"

/// \defgroup UPG Upgrade Manager
/// @{

/// \brief upgrade stages
/// ..WARNING... this should be extended in the end for maintaining backward
/// and forward compatibility b/w upgrademgr and services.
/// also the stage name mentioned here should match with the upgrade.json. so
/// any change in that should be updated in the json.
#define UPG_STAGE_ENTRIES(E)                                        \
    /** invalid */                                                  \
    E(UPG_STAGE_NONE,               0,      "invalid")              \
    /**  compat checks  */                                          \
    E(UPG_STAGE_COMPAT_CHECK,       1,      "compatcheck")          \
    /**  start an upgrade */                                        \
    E(UPG_STAGE_START,              2,      "start")                \
    /** backup states for an upgrade */                             \
    E(UPG_STAGE_BACKUP,             3,      "backup")               \
    /** prepare for an upgrade */                                   \
    E(UPG_STAGE_PREPARE,            4,      "prepare")              \
    /** config replay and operational state sync */                 \
    E(UPG_STAGE_SYNC,               5,      "sync")                 \
    /** quiescing before switch to the new version */               \
    E(UPG_STAGE_PREP_SWITCHOVER,    6,      "prepare_switchover")   \
    /** switch to the new version   */                              \
    E(UPG_STAGE_SWITCHOVER,         7,      "switchover")           \
    /** readiness check of new version */                           \
    E(UPG_STAGE_READY,              8,      "ready")                \
    /** respawn the existing version */                             \
    E(UPG_STAGE_RESPAWN,            9,      "respawn")              \
    /** rollback to the previous version */                         \
    E(UPG_STAGE_ROLLBACK,           10,     "rollback")             \
    /** abort the on-going upgrade by undoing */                    \
    E(UPG_STAGE_REPEAL,             11,     "repeal")               \
    /** completion of an upgrade */                                 \
    E(UPG_STAGE_FINISH,             12,     "finish")               \
    /** exit from upgrade */                                        \
    E(UPG_STAGE_EXIT,               13,     "exit")                 \
    /** invalid */                                                  \
    E(UPG_STAGE_MAX,                14,     "max-invalid")

SDK_DEFINE_ENUM(upg_stage_t, UPG_STAGE_ENTRIES)
SDK_DEFINE_ENUM_TO_STR(upg_stage_t, UPG_STAGE_ENTRIES)

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
#define UPG_STATUS_ENTRIES(E)                       \
    /** operation successful    */                  \
    E(UPG_STATUS_OK,        0, "ok")                \
    /** operation failed, but system is stable */   \
    E(UPG_STATUS_FAIL,      1, "fail")              \
    /** operation failed, and system is unstable */ \
    E(UPG_STATUS_CRITICAL,  2, "fail-critical")     \

SDK_DEFINE_ENUM(upg_status_t, UPG_STATUS_ENTRIES)
SDK_DEFINE_ENUM_TO_STR(upg_status_t, UPG_STATUS_ENTRIES)
#undef UPG_STATUS_ENTRIES

/// \brief upgrade event msg
/// as the upgrade mgr need to communicate with old and new images,
/// we should not add anything in the middle.
/// TODO: should i convert to protobuf and send
typedef struct upg_event_msg_s {
    upg_stage_t    stage;             ///< request stage
    sdk::platform::upg_mode_t mode;   ///< upgrade mode
    upg_status_t   rsp_status;        ///< response status
    char           rsp_svc_name[64];  ///< response service's name
    uint32_t       rsp_svc_ipc_id;    ///< response services's ipc id. can be used
                                      ///< to send unicast ipc to this thread
    uint64_t       rsp_ev_bitmap;     ///< event supported by response thread
                                      ///< can be used by upgmgr to send an
                                      ///< event to the service
    // TODO other infos
} __PACK__ upg_event_msg_t;

// trace utilities
// stage to string
static inline const char *
upg_stage2str (upg_stage_t stage)
{
    return UPG_STAGE_ENTRIES_str(stage);
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
