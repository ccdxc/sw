// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains common message headers, types etc. that are global across
/// threads and processes
/// WARNING: this must be a C file, not C++
//----------------------------------------------------------------------------

#ifndef __CORE_MSG_HPP__
#define __CORE_MSG_HPP__

#ifdef __cplusplus
extern "C" {
#endif

#include "nic/apollo/framework/api.h"
#include "nic/apollo/framework/api.hpp"

/// message types
typedef enum pds_msg_type_s {
    PDS_MSG_TYPE_NONE,
    PDS_MSG_TYPE_CFG,      ///< config type message
    PDS_MSG_TYPE_CMD,      ///< any CLI commands
    PDS_MSG_TYPE_EVENT,    ///< event type message
} pds_msg_type_t;

/// unique message identifiers
typedef enum pds_msg_id_s {
    PDS_MSG_ID_NONE,
    PDS_MSG_ID_MIN = PDS_MSG_ID_NONE,

    /// config/policy message identifiers
    PDS_CFG_MSG_ID_NONE,
    /// any device/global configuration
    PDS_CFG_MSG_ID_DEVICE,
    /// vnic related configuration
    PDS_CFG_MSG_ID_VNIC,
    /// subnet related configuration
    PDS_CFG_MSG_ID_SUBNET,
    /// DHCP poliy configuration
    PDS_CFG_MSG_ID_DHCP,
    /// NAT port block configuration
    PDS_CFG_MSG_ID_NAT_PORT_BLOCK,

    // TODO: should we punt event/alerts to shm directly
    PDS_CMD_MSG_ID_NONE,
    PDS_CMD_MSG_FLOW_CLEAR,
    PDS_MSG_ID_MAX,
} pds_msg_id_t;

/// vnic configuration
typedef struct pds_vnic_cfg_msg_s {
} pds_vnic_cfg_msg_t;

/// subnet configuration
typedef struct pds_subnet_cfg_msg_s {
} pds_subnet_cfg_msg_t;

/// DHCP proxy configuration
typedef struct pds_dhcp_proxy_cfg_msg_s {
} pds_dhcp_proxy_cfg_msg_t;

/// DHCP relay configuration
typedef struct pds_dhcp_relay_cfg_msg_s {
} pds_dhcp_relay_cfg_msg_t;

/// NAT port block configuration
typedef struct pds_nat_port_block_cfg_msg_s {
} pds_nat_port_block_cfg_msg_t;

/// configuration message sent or received
typedef struct pds_cfg_msg_s {
    /// API operation
    api_op_t op;
    /// API object id
    obj_id_t obj_id;
    /// msg contents
    union {
        pds_vnic_cfg_msg_t vnic;
        pds_subnet_cfg_msg_t subnet;
        pds_dhcp_proxy_cfg_msg_t dhcp_proxy;
        pds_dhcp_relay_cfg_msg_t dhcp_relay;
        pds_nat_port_block_cfg_msg_t nat_port_block;
    };
} pds_cfg_msg_t;

/// top level PDS message structure for all types of messages
typedef struct pds_msg_s {
    pds_msg_type_t type;       ///< type of the message
    pds_msg_id_t   id;         ///< unique id of the msg
    uint8_t        data[0];    ///< msg type + id specific data
} pds_msg_t;

#ifdef __cplusplus
}
#endif

#endif    // __CORE_MSG_HPP__
