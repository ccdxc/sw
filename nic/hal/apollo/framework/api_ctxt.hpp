/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    api_ctxt.hpp
 *
 * @brief   Internal API context information
 */

#if !defined (__API_CTXT_HPP__)
#define __API_CTXT_HPP__

#include "nic/hal/apollo/framework/api.hpp"
#include "nic/hal/apollo/api/vcn.hpp"
#include "nic/hal/apollo/api/subnet.hpp"
#include "nic/hal/apollo/api/vnic.hpp"

namespace api {

/**< API specific parameters */
typedef union api_params_u {
    oci_vcn_key_t       vcn_key;
    oci_vcn_t           vcn_info;
    oci_subnet_key_t    subnet_key;
    oci_subnet_t        subnet_info;
    oci_vnic_key_t      vnic_key;
    oci_vnic_t          vnic_info;
} api_params_t;

/**
 * @brief    per API context maintained by the framework while processing
 *           the API
 */
typedef struct api_ctxt_s {
    api_op_t        api_op;         /**< api operation */
    obj_id_t        obj_id;         /**< object identifier */
    api_params_t    *api_params;    /**< API specific params */
} api_ctxt_t;

}    // namespace api

using api::api_ctxt_t;
using api::api_params_t;

#endif    /** __API_CTXT_HPP__ */

