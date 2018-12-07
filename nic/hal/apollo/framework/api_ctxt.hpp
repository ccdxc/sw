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

/**
 * @brief    per API context maintained by the framework while processing
 *           the API
 */
typedef struct api_ctxt_s {
    api_op_t    api_op;           /**< api operation */
    obj_id_t    obj_id;           /**< object identifier */
    api_base    *curr_obj;        /**< current version of the object */
    api_base    *new_obj;         /**< modified or next version of the object */
    /**< API specific parameters */
    union {
        oci_vcn_t           vcn_create;
        oci_vcn_key_t       vcn_delete;
        oci_subnet_t        subnet_create;
        oci_subnet_key_t    subnet_delete;
        oci_vnic_t          vnic_create;
        oci_vnic_key_t      vnic_delete;
    };
} api_ctxt_t;

}    // namespace api

using api::api_ctxt_t;
using api::api_op_t;
using api::obj_id_t;

#endif    /** __API_CTXT_HPP__ */

