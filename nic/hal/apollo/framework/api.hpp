/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    api.hpp
 *
 * @brief   basic types for API processing
 */

#if !defined (__API_HPP__)
#define __API_HPP__

namespace api {

/**< forward declarations */
typedef struct api_ctxt_s api_ctxt_t;
typedef struct obj_ctxt_s obj_ctxt_t;

/**
 * @brief    API operation
 */
typedef enum api_op_e {
    API_OP_NONE,
    API_OP_CREATE,
    API_OP_DELETE,
    API_OP_UPDATE,
    API_OP_INVALID,
} api_op_t;

/**
 * @brief    object identifiers
 */
typedef enum obj_id_e {
    OBJ_ID_NONE,
    OBJ_ID_SWITCHPORT,
    OBJ_ID_VCN,
    OBJ_ID_SUBNET,
    OBJ_ID_TEP,
    OBJ_ID_VNIC,
    OBJ_ID_ROUTE,
    OBJ_ID_SECURITY_RULES,
    OBJ_ID_MAPPING,
    OBJ_ID_MAX,
} obj_id_t;

}    // namespace api

using api::api_op_t;
using api::obj_id_t;
using api::api_ctxt_t;
using api::obj_ctxt_t;

#endif    /** __API_HPP__ */
