// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains common message headers, types etc. that are global across
/// threads and processes
/// WARNING: this must be a C file, not C++
//----------------------------------------------------------------------------

#ifndef __CORE_MSG_H__
#define __CORE_MSG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nic/apollo/framework/api.h"

/// IPC endpoints
/// IPC endpoints internal to PDS agent/HAL use their unique thread ids as their
/// IPC (client/server) identifier
/// IPC endpoints external to PDS agent/HAL can use the enums defined below to
/// identify themselves and register for (sync/async events)
/// IPC id split currrently is as below:
/// [PDS_THREAD_ID_NONE - PDS_THREAD_ID_MAX] are for PDS HAL threads
/// [PDS_THREAD_ID_MAX + 1 - PDS_AGENT_THREAD_ID_MAX]
typedef enum pds_ipc_id_s {
    PDS_IPC_ID_NONE = 48,
    PDS_IPC_ID_MIN  = PDS_IPC_ID_NONE,
    PDS_IPC_ID_VPP  = PDS_IPC_ID_MIN,
} pds_ipc_id_t;

/// message types
typedef enum pds_msg_type_s {
    PDS_MSG_TYPE_NONE,
    PDS_MSG_TYPE_CFG,      ///< config type message
    PDS_MSG_TYPE_CMD,      ///< any CLI commands
    PDS_MSG_TYPE_EVENT,    ///< event type message

    PDS_MSG_TYPE_MAX
} pds_msg_type_t;

/// unique message identifiers
typedef enum pds_msg_id_s {
    PDS_MSG_ID_NONE,
    PDS_MSG_ID_MIN = PDS_MSG_ID_NONE,

    /// config/policy message identifiers
    PDS_CFG_MSG_ID_NONE,
    /// any device/global configuration
    PDS_CFG_MSG_ID_DEVICE,
    /// vpc related configuration
    PDS_CFG_MSG_ID_VPC,
    /// vnic related configuration
    PDS_CFG_MSG_ID_VNIC,
    /// subnet related configuration
    PDS_CFG_MSG_ID_SUBNET,
    /// NAT port block configuration
    PDS_CFG_MSG_ID_NAT_PORT_BLOCK,
    /// DHCP (suppresion) policy configuration
    PDS_CFG_MSG_ID_DHCP_POLICY,
    /// DHCP relay configuration
    PDS_CFG_MSG_ID_DHCP_RELAY,
    /// security profile configuration
    PDS_CFG_MSG_ID_SECURITY_PROFILE,

    // TODO: should we punt event/alerts to shm directly
    PDS_CMD_MSG_ID_NONE,
    PDS_CMD_MSG_FLOW_CLEAR,
    PDS_MSG_ID_MAX,
} pds_msg_id_t;

#ifdef __cplusplus
}
#endif

#endif    // __CORE_MSG_H__
