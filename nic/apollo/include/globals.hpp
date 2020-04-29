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

#include "nic/sdk/include/sdk/globals.hpp"

namespace {

/// grpc port for pds agent
#define PDS_GRPC_PORT_API                 50054 //11357
/// grpc port for upgrade manager
#define PDS_GRPC_PORT_UPGMGR              51512 //11358
/// grpc port for operd
#define PDS_GRPC_PORT_OPERD               11359
/// grpc port for operd default plugin
#define PDS_GRPC_PORT_OPERD_PEN_PLUGIN    11360

/// thread ids
/// [PDS_THREAD_ID_MIN - PDS_THREAD_ID_MAX] are for PDS HAL threads
/// [PDS_AGENT_THREAD_ID_MIN - PDS_AGENT_THREAD_ID_MAX] are for PDS Agent threads
/// IPC endpoints internal to PDS agent/HAL use their unique thread ids as their
/// IPC (client/server) identifier. so reserving the thread ids from ipc ids
typedef enum pds_thread_id_s {
    PDS_THREAD_ID_MIN       = (SDK_IPC_ID_MAX + 1),
    PDS_THREAD_ID_MAX       = (PDS_THREAD_ID_MIN + 15),
    PDS_AGENT_THREAD_ID_MIN = (PDS_THREAD_ID_MAX + 1),
    PDS_AGENT_THREAD_ID_MAX = (PDS_AGENT_THREAD_ID_MIN + 7)
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
} pds_ipc_id_t;

/// IPC broadcast event ids
/// IPC broadcast events generated & subscribed by PDS agent/HAL threads
/// internally uses ids from the HAL_MIN
/// IPC endpoints external PDS agent uses event ids after HAL max
typedef enum pds_ipc_event_id_s {
    PDS_IPC_EVENT_ID_HAL_MIN = SDK_IPC_EVENT_ID_MAX + 1,
    PDS_IPC_EVENT_ID_HAL_MAX = (PDS_IPC_EVENT_ID_HAL_MIN + 23),
    PDS_IPC_EVENT_ID_VPP_MIN = PDS_IPC_EVENT_ID_HAL_MAX + 1,
    PDS_IPC_EVENT_ID_VPP_MAX = PDS_IPC_EVENT_ID_VPP_MIN + 3,
} pds_ipc_event_id_t;

}    // namespace

#endif    // __INCLUDE_GLOBALS_HPP__
