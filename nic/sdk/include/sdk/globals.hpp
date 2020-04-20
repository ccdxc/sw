//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines the globals for sdk
///
//----------------------------------------------------------------------------

#ifndef __SDK_GLOBALS_HPP__
#define __SDK_GLOBALS_HPP__

/// \brief ipc endpoint ids reserved for sdk processes
typedef enum sdk_ipc_id_s {
    SDK_IPC_ID_MIN          = 0,
    SDK_IPC_ID_UPGMGR       = SDK_IPC_ID_MIN + 1,
    SDK_IPC_ID_PCIEMGR      = SDK_IPC_ID_MIN + 2,
    SDK_IPC_ID_MAX          = SDK_IPC_ID_MIN + 31,
} sdk_ipc_id_t;

/// \brief ipc broadcast event ids reserved for sdk processes
typedef enum sdk_ipc_event_id_s {
    SDK_IPC_EVENT_ID_MIN            = 0,
    // ..WARNING ... during hitless upgrade, previous/new version of upgrade mgr
    // may need to communicate with new/previous version of processes.
    // so changing this id will result in upgrade backward/forward
    // compatibility failures.
    SDK_IPC_EVENT_ID_UPGMGR_MIN     = SDK_IPC_EVENT_ID_MIN + 1,
    SDK_IPC_EVENT_ID_UPGMGR_MAX     = SDK_IPC_EVENT_ID_UPGMGR_MIN + 23,
    SDK_IPC_EVENT_ID_MAX            = SDK_IPC_EVENT_ID_UPGMGR_MAX,
} sdk_ipc_event_id_t;

/// \brief ipc unicast event ids reserved for sdk processes
typedef enum sdk_ipc_msg_id_s {
    SDK_IPC_MSG_ID_MIN            = 0,
    // ..WARNING ... during hitless upgrade, previous/new version of upgrade mgr
    // may need to communicate with new/previous version of processes.
    // so changing this id will result in upgrade backward/forward
    // compatibility failures.
    SDK_IPC_MSG_ID_UPGMGR_MIN     = SDK_IPC_MSG_ID_MIN + 1,
    SDK_IPC_MSG_ID_UPGMGR_MAX     = SDK_IPC_MSG_ID_UPGMGR_MIN + 23,
    SDK_IPC_MSG_ID_MAX            = SDK_IPC_MSG_ID_UPGMGR_MAX,
} sdk_ipc_msg_id_t;

/// \brief module ids reserved for sdk processes
typedef enum sdk_mod_id_e {
    SDK_MOD_ID_MIN   = 0,
    SDK_MOD_ID_SDK   = SDK_MOD_ID_MIN + 1,  ///< generic SDK module id
    SDK_MOD_ID_HMON,                        ///< module id for system monitoring
    SDK_MOD_ID_INTR,                        ///< module id for system interrupts
    SDK_MOD_ID_LINK,                        ///< module id for port links
    SDK_MOD_ID_MAX
} sdk_mod_id_t;

#endif   // __SDK_GLOBALS_HPP__
