/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    api_ctxt.hpp
 *
 * @brief   Internal API context information
 */

#if !defined (__API_CTXT_HPP__)
#define __API_CTXT_HPP__

#include "nic/hal/apollo/api/vcn.hpp"
#include "nic/hal/apollo/api/subnet.hpp"
#include "nic/hal/apollo/api/vnic.hpp"

namespace api {

/**
 * @brief    API operation
 */
typedef enum api_op_e {
    API_OP_NONE,
    API_OP_CREATE,
    API_OP_DELETE,
    API_OP_UPDATE,
    API_OP_GET,
} api_op_t;

/**
 * @brief    API identifiers
 */
typedef enum api_id_e {
    API_ID_NONE,
    API_ID_VCN_CREATE,
    API_ID_VCN_DELETE,
    API_ID_VCN_UPDATE,
    API_ID_VCN_GET,
    API_ID_SUBNET_CREATE,
    API_ID_SUBNET_DELETE,
    API_ID_SUBNET_UPDATE,
    API_ID_SUBNET_GET,
    API_ID_VNIC_CREATE,
    API_ID_VNIC_DELETE,
    API_ID_VNIC_UPDATE,
    API_ID_VNIC_GET,
    API_ID_ROUTE_CREATE,
    API_ID_ROUTE_DELETE,
    API_ID_ROUTE_UPDATE,
    API_ID_ROUTE_GET,
    API_ID_MAX,
} api_id_t;

/**
 * @brief    per API context maintained by the framework while processing
 *           the API
 */
typedef struct api_ctxt_s {
    api_op_t    op;           /**< api operation */
    api_id_t    id;           /**< api identifier */
    /**< API specific parameters */
    union {
        oci_vcn_t           vcn_create;
        oci_vcn_key_t       vcn_delete;
        oci_subnet_t        subnet_create;
        oci_subnet_key_t    subnet_delete;
        oci_vnic_t          vnic_create;
        oci_vnic_key_t      vnic_delete;
    } params;
} api_ctxt_t;

}    // namespace api

#endif    /** __API_CTXT_HPP__ */

