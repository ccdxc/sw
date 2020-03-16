//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines basic types for API processing
/// WARNING: this must be a C file, not C++
//----------------------------------------------------------------------------

#ifndef __FRAMEWORK_API_H__
#define __FRAMEWORK_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/// \brief API operation
typedef enum api_op_e {
    API_OP_NONE,       ///< None
    API_OP_CREATE,     ///< Create
    API_OP_DELETE,     ///< Delete
    API_OP_UPDATE,     ///< Update
    API_OP_INVALID,    ///< Invalid
} api_op_t;

/// \brief Object identifiers
typedef enum obj_id_e {
    OBJ_ID_NONE               = 0,     ///< None
    OBJ_ID_BATCH              = 1,     ///< batch
    OBJ_ID_DEVICE             = 2,     ///< device
    OBJ_ID_PORT               = 3,     ///< port
    OBJ_ID_IF                 = 4,     ///< interface
    OBJ_ID_VPC                = 5,     ///< vpc
    OBJ_ID_SUBNET             = 6,     ///< subnet
    OBJ_ID_TEP                = 7,     ///< TEP
    OBJ_ID_VNIC               = 8,     ///< vNIC
    OBJ_ID_MAPPING            = 9,     ///< mapping
    OBJ_ID_ROUTE_TABLE        = 10,    ///< route table
    OBJ_ID_POLICY             = 11,    ///< policy
    OBJ_ID_MIRROR_SESSION     = 12,    ///< mirror session
    OBJ_ID_METER              = 13,    ///< meter
    OBJ_ID_TAG                = 14,    ///< (service) tag
    OBJ_ID_SVC_MAPPING        = 15,    ///< service mapping
    OBJ_ID_VPC_PEER           = 16,    ///< vpc peer
    OBJ_ID_NEXTHOP            = 17,    ///< nexthop
    OBJ_ID_NEXTHOP_GROUP      = 18,    ///< nexthop group
    OBJ_ID_POLICER            = 19,    ///< policer
    OBJ_ID_NAT_PORT_BLOCK     = 20,    ///< NAT port block
    OBJ_ID_DHCP_POLICY        = 21,    ///< DHCP relay/proxy policy
    OBJ_ID_SECURITY_PROFILE   = 22,    ///< Security profile
    OBJ_ID_MAX
} obj_id_t;

#ifdef __cplusplus
}
#endif

#endif    // __FRAMEWORK_API_HPP__
