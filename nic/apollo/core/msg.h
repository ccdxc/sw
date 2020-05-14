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
#include "nic/apollo/include/globals.hpp"

/// message types
typedef enum pds_msg_type_s {
    PDS_MSG_TYPE_NONE = (SDK_IPC_MSG_ID_MAX + 1),
    PDS_MSG_TYPE_CFG_OBJ_SET,                     ///< config create/update msg
    PDS_MSG_TYPE_CFG_OBJ_GET,                     ///< config read message
    PDS_MSG_TYPE_CFG_OBJ_GET_ALL,                 ///< config read all message
    PDS_MSG_TYPE_CMD,                             ///< miscellaneous cmd msgs
    PDS_MSG_TYPE_EVENT,                           ///< event type message

    PDS_MSG_TYPE_MAX
} pds_msg_type_t;

/// unique cmd message identifiers
typedef enum pds_cmd_msg_id_s {
    // TODO: should we punt event/alerts to shm directly
    PDS_CMD_MSG_ID_NONE,
    PDS_CMD_MSG_ID_MIN = PDS_CMD_MSG_ID_NONE,
    // clear flows matching key
    PDS_CMD_MSG_FLOW_CLEAR,
    /// get statistics of specified VNIC
    PDS_CMD_MSG_VNIC_STATS_GET,
    PDS_MSG_ID_MAX,
} pds_cmd_msg_id_t;

#ifdef __cplusplus
}
#endif

#endif    // __CORE_MSG_H__
