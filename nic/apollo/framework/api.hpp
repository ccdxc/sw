//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines basic types for API processing
///
//----------------------------------------------------------------------------

#ifndef __FRAMEWORK_API_HPP__
#define __FRAMEWORK_API_HPP__

#include "nic/apollo/api/include/pds.hpp"

namespace api {

// forward declarations
typedef struct api_ctxt_s api_ctxt_t;
typedef struct obj_ctxt_s obj_ctxt_t;

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
    OBJ_ID_DEVICE,         ///< Device
    OBJ_ID_VPC,            ///< VPC
    OBJ_ID_SUBNET,         ///< Subnet
    OBJ_ID_TEP,            ///< TEP
    OBJ_ID_VNIC,           ///< VNIC
    OBJ_ID_MAPPING,        ///< Mapping
    OBJ_ID_ROUTE_TABLE,    ///< Route table
    OBJ_ID_POLICY,         ///< Policy
    OBJ_ID_MIRROR_SESSION, ///< Mirror session
    OBJ_ID_METER,          ///< Meter
    OBJ_ID_TAG,            ///< (Service) tag
    OBJ_ID_MAX,
} obj_id_t;

/// \brief    return current epoch value
pds_epoch_t get_current_epoch(void);

}    // namespace api

using api::api_op_t;
using api::obj_id_t;
using api::api_ctxt_t;
using api::obj_ctxt_t;

#endif    // __FRAMEWORK_API_HPP__
