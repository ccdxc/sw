//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines basic types for API processing
/// WARNING: this must be a C file, not C++
///
//----------------------------------------------------------------------------

#ifndef __FRAMEWORK_API_H__
#define __FRAMEWORK_API_H__

#include "nic/apollo/api/include/pds.hpp"

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
    OBJ_ID_NONE,           ///< None
    OBJ_ID_BATCH,          ///< batch
    OBJ_ID_DEVICE,         ///< device
    OBJ_ID_PORT,           ///< port
    OBJ_ID_IF,             ///< interface
    OBJ_ID_VPC,            ///< vpc
    OBJ_ID_SUBNET,         ///< subnet
    OBJ_ID_TEP,            ///< TEP
    OBJ_ID_VNIC,           ///< vNIC
    OBJ_ID_MAPPING,        ///< mapping
    OBJ_ID_ROUTE_TABLE,    ///< route table
    OBJ_ID_POLICY,         ///< policy
    OBJ_ID_MIRROR_SESSION, ///< mirror session
    OBJ_ID_METER,          ///< meter
    OBJ_ID_TAG,            ///< (service) tag
    OBJ_ID_SVC_MAPPING,    ///< service mapping
    OBJ_ID_VPC_PEER,       ///< vpc peer
    OBJ_ID_NEXTHOP,        ///< nexthop
    OBJ_ID_NEXTHOP_GROUP,  ///< nexthop group
    OBJ_ID_MAX,
} obj_id_t;

#endif    // __FRAMEWORK_API_HPP__
