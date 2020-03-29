//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains global macros
/// APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_GLOBALS_HPP__
#define __INCLUDE_GLOBALS_HPP__

namespace {

/// grpc port for upgrade manager
#define GRPC_UPGRADE_PORT  8888

/// thread ids
/// [PDS_THREAD_ID_MIN - PDS_THREAD_ID_MAX] are for PDS HAL threads
/// [PDS_AGENT_THREAD_ID_MIN - PDS_AGENT_THREAD_ID_MAX] are for PDS Agent threads
typedef enum pds_thread_id_s {
    PDS_THREAD_ID_MIN = 0,
    PDS_THREAD_ID_MAX = 31,
    PDS_AGENT_THREAD_ID_MIN = (PDS_THREAD_ID_MAX + 1),
    PDS_AGENT_THREAD_ID_MAX = 47
} pds_thread_id_t;

/// IPC endpoints
/// IPC endpoints internal to PDS agent/HAL use their unique thread ids as their
/// IPC (client/server) identifier
/// IPC endpoints external to PDS agent/HAL can use the enums defined below to
/// identify themselves and register for (sync/async events)
/// IPC id split currrently is as below:
/// [PDS_THREAD_ID_NONE - PDS_THREAD_ID_MAX] are for PDS HAL threads
/// [PDS_THREAD_ID_MAX + 1 - PDS_AGENT_THREAD_ID_MAX]
typedef enum pds_ipc_id_s {
    PDS_IPC_ID_MIN      = (PDS_AGENT_THREAD_ID_MAX + 1),
    PDS_IPC_ID_VPP      = PDS_IPC_ID_MIN,
    PDS_IPC_ID_UPGRADE,
} pds_ipc_id_t;

/// IPC broadcast message ids
/// PDS_IPC_MSG_ID_HAL_MAX message ids as reserved for pds agent/hal threads
/// upgrade message id should be unique across processes to receive broadcast
/// messages from upgrade manager. modifying this id will break ISSU
typedef enum pds_ipc_msg_id_s {
    PDS_IPC_MSG_ID_HAL_MIN = 0,
    PDS_IPC_MSG_ID_HAL_MAX = 40,
    PDS_IPC_MSG_ID_UPGRADE = (PDS_IPC_MSG_ID_HAL_MAX + 1),
    PDS_IPC_MSG_ID_UPG_COMPAT_CHECK,
    PDS_IPC_MSG_ID_UPG_START,
    PDS_IPC_MSG_ID_UPG_BACKUP,
    PDS_IPC_MSG_ID_UPG_PREPARE,
    PDS_IPC_MSG_ID_UPG_READY,
    PDS_IPC_MSG_ID_UPG_SYNC,
    PDS_IPC_MSG_ID_UPG_PREPARE_SWITCHOVER,
    PDS_IPC_MSG_ID_UPG_SWITCHOVER,
    PDS_IPC_MSG_ID_UPG_ROLLBACK,
    PDS_IPC_MSG_ID_UPG_REPEAL,
    PDS_IPC_MSG_ID_UPG_FINISH,
    PDS_IPC_MSG_ID_UPG_EXIT,
    PDS_IPC_MSG_ID_UPGRADE_MAX = 60,
} pds_ipc_msg_id_t;

}    // namespace

#endif    // __INCLUDE_GLOBALS_HPP__
