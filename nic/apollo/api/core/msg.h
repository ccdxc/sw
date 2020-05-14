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
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds_dhcp.hpp"
#include "nic/apollo/api/include/pds_nat.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/api/include/pds_flow.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/// configuration create, update, delete message structures
typedef struct pds_device_msg_s {
    pds_obj_key_t key;
    pds_device_spec_t spec;
} pds_device_msg_t;

/// device configuration
typedef struct pds_device_cfg_msg_s {
    union {
        pds_obj_key_t key;
        pds_device_msg_t spec;
    };
} pds_device_cfg_msg_t;

/// vpc configuration
typedef struct pds_vpc_cfg_msg_s {
    union {
        pds_obj_key_t key;
        pds_vpc_spec_t spec;
    };
    pds_vpc_status_t status;
} pds_vpc_cfg_msg_t;

/// vnic configuration
typedef struct pds_vnic_cfg_msg_s {
    union {
        pds_obj_key_t key;
        pds_vnic_spec_t spec;
    };
    pds_vnic_status_t status;
} pds_vnic_cfg_msg_t;

/// subnet configuration
typedef struct pds_subnet_cfg_msg_s {
    union {
        pds_obj_key_t key;
        pds_subnet_spec_t spec;
    };
    pds_subnet_status_t status;
} pds_subnet_cfg_msg_t;

/// NAT port block configuration
typedef struct pds_nat_port_block_cfg_msg_s {
    union {
        pds_obj_key_t key;
        pds_nat_port_block_spec_t spec;
    };
    pds_nat_port_block_status_t status;
    pds_nat_port_block_stats_t stats;
} pds_nat_port_block_cfg_msg_t;

/// DHCP policy configuration
typedef struct pds_dhcp_policy_cfg_msg_s {
    union {
        pds_obj_key_t key;
        pds_dhcp_policy_spec_t spec;
    };
    pds_dhcp_policy_status_t status;
} pds_dhcp_policy_cfg_msg_t;

/// security profile configuration
typedef struct pds_security_profile_cfg_msg_s {
    union {
        pds_obj_key_t key;
        pds_security_profile_spec_t spec;
    };
    pds_security_profile_status_t status;
} pds_security_profile_cfg_msg_t;

/// configuration message structure for create/update/delete operations
typedef struct pds_cfg_msg_s {
    /// API operation
    api_op_t op;
    /// API object id
    obj_id_t obj_id;
    /// msg contents
    union {
        pds_device_cfg_msg_t device;
        pds_vpc_cfg_msg_t vpc;
        pds_vnic_cfg_msg_t vnic;
        pds_subnet_cfg_msg_t subnet;
        pds_dhcp_policy_cfg_msg_t dhcp_policy;
        pds_nat_port_block_cfg_msg_t nat_port_block;
        pds_security_profile_cfg_msg_t security_profile;
    };
} pds_cfg_msg_t;

/// configuration read message structures
/// configuration object read request message
typedef struct pds_cfg_get_req_s {
    /// API object id
    obj_id_t obj_id;
    /// object key
    pds_obj_key_t key;
} pds_cfg_get_req_t;

typedef struct pds_cfg_get_rsp_s {
    /// read result
    uint32_t status;   ///< sdk::sdk_ret_t cast as u32
    /// API object id
    obj_id_t obj_id;
    /// object cfg, status and stats
    union {
        pds_dhcp_policy_info_t dhcp_policy;
        pds_nat_port_block_info_t nat_port_block;
        pds_security_profile_info_t security_profile;
    };
} pds_cfg_get_rsp_t;

typedef struct pds_cfg_get_all_req_s {
    /// API object id
    obj_id_t obj_id;
} pds_cfg_get_all_req_t;

typedef struct pds_cfg_get_all_rsp_s {
    /// read result
    uint32_t status;   ///< sdk::sdk_ret_t cast as u32
    /// count of objects contained
    uint32_t count;
    union {
        pds_dhcp_policy_info_t dhcp_policy[0];
        pds_nat_port_block_info_t nat_port_block[0];
        pds_security_profile_info_t security_profile[0];
    };
} pds_cfg_get_all_rsp_t;

/// command message structures
typedef struct pds_flow_clear_cmd_msg_s {
    pds_flow_key_t key;
} pds_flow_clear_cmd_msg_t;

typedef struct pds_vnic_stats_get_cmd_msg_s {
    uint16_t vnic_hw_id;
} pds_vnic_stats_get_cmd_msg_t;

/// command message sent or received
typedef struct pds_cmd_msg_s {
    pds_cmd_msg_id_t   id;            ///< unique id of the msg
    /// msg contents
    union {
        pds_flow_clear_cmd_msg_t flow_clear;         ///< FLOW_CLEAR
        pds_vnic_stats_get_cmd_msg_t vnic_stats_get; ///< VNIC_STATS_GET
    };
} pds_cmd_msg_t;

/// reply message sent for cmd message
typedef struct pds_cmd_rsp_s {
    uint32_t status;   ///< sdk::sdk_ret_t cast as u32
    union {
        pds_vnic_stats_t vnic_stats; ///< cmd id: PDS_CMD_MSG_VNIC_STATS_GET
    };
} pds_cmd_rsp_t;

/// top level PDS message structure for all types of messages
typedef struct pds_msg_s {
    union {
        pds_cfg_msg_t cfg_msg;    ///< cfg msg
        pds_cmd_msg_t cmd_msg;    ///< cmd msg
    };
} pds_msg_t;

/// batch of PDS messages
typedef struct pds_msg_list_s {
    pds_msg_type_t type; ///< type of the message
    pds_epoch_t epoch;   ///< config epoch (for cfg msgs) or PDS_EPOCH_INVALID
    uint32_t num_msgs;   ///< number of messages in this batch
    pds_msg_t msgs[0];   ///< batch of messages
} pds_msg_list_t;

#ifdef __cplusplus
}
#endif

#endif    // __API_CORE_MSG_H__
