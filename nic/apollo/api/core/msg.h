// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains common message headers, types etc. that are global across
/// threads and processes
/// WARNING: this must be a C file, not C++
//----------------------------------------------------------------------------

#ifndef __API_CORE_MSG_H__
#define __API_CORE_MSG_H__

#include "nic/apollo/framework/api.h"
#include "nic/apollo/core/msg.h"
#include "nic/apollo/api/include/pds_dhcp.hpp"
#include "nic/apollo/api/include/pds_nat.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/// vnic configuration
typedef struct pds_vnic_cfg_msg_s {
    union {
        pds_vnic_key_t key;
        pds_vnic_spec_t spec;
    };
    pds_vnic_status_t status;
} pds_vnic_cfg_msg_t;

/// subnet configuration
typedef struct pds_subnet_cfg_msg_s {
    union {
        pds_subnet_key_t key;
        pds_subnet_spec_t spec;
    };
    pds_subnet_status_t status;
} pds_subnet_cfg_msg_t;

/// DHCP policy configuration
typedef struct pds_dhcp_policy_cfg_msg_s {
    union {
        pds_dhcp_policy_key_t key;
        pds_dhcp_policy_spec_t spec;
    };
} pds_dhcp_policy_cfg_msg_t;

/// DHCP relay configuration
typedef struct pds_dhcp_relay_cfg_msg_s {
    union {
        pds_dhcp_relay_key_t key;
        pds_dhcp_relay_spec_t spec;
    };
} pds_dhcp_relay_cfg_msg_t;

/// NAT port block configuration
typedef struct pds_nat_port_block_cfg_msg_s {
    union {
        pds_nat_port_block_key_t key;
        pds_nat_port_block_spec_t spec;
    };
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
        pds_dhcp_relay_cfg_msg_t dhcp_relay;
        pds_dhcp_policy_cfg_msg_t dhcp_policy;
        pds_nat_port_block_cfg_msg_t nat_port_block;
    };
} pds_cfg_msg_t;

/// top level PDS message structure for all types of messages
typedef struct pds_msg_s {
    pds_msg_type_t type;          ///< type of the message
    pds_msg_id_t   id;            ///< unique id of the msg
    union {
        pds_cfg_msg_t cfg_msg;    ///< cfg msg
    };
} pds_msg_t;

/// batch of PDS messages
typedef struct pds_msg_list_s {
    uint32_t num_msgs;
    pds_msg_t msgs[0];
} pds_msg_list_t;

#ifdef __cplusplus
}
#endif

#endif    // __API_CORE_MSG_H__
