/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    api_ctxt.hpp
 *
 * @brief   Internal API context information
 */

#if !defined (__API_CTXT_HPP__)
#define __API_CTXT_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/api/switchport.hpp"   // TODO: why not just include oci_xxx here ?
#include "nic/apollo/api/tep.hpp"
#include "nic/apollo/api/vcn.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/vnic.hpp"
#include "nic/apollo/api/mapping.hpp"
#include "nic/apollo/api/route.hpp"
#include "nic/apollo/include/api/oci_security_policy.hpp"

namespace api {

/**< API specific parameters */
typedef union api_params_u {
    oci_switchport_t             switchport_info;
    oci_tep_key_t                tep_key;
    oci_tep_t                    tep_info;
    oci_vcn_key_t                vcn_key;
    oci_vcn_t                    vcn_info;
    oci_subnet_key_t             subnet_key;
    oci_subnet_t                 subnet_info;
    oci_vnic_key_t               vnic_key;
    oci_vnic_t                   vnic_info;
    oci_mapping_key_t            mapping_key;
    oci_mapping_t                mapping_info;
    oci_route_table_key_t        route_table_key;
    oci_route_table_t            route_table_info;
    oci_security_policy_key_t    security_policy_key;
    oci_security_policy_t        security_policy_info;
} api_params_t;

/**< @brief    per API context maintained by framework while processing */
typedef struct api_ctxt_s {
    api_op_t        api_op;         /**< api operation */
    obj_id_t        obj_id;         /**< object identifier */
    api_params_t    *api_params;    /**< API specific params */
} api_ctxt_t;

slab *api_params_slab(void);
static inline api_params_t *
api_params_alloc (obj_id_t obj_id, api_op_t api_op)
{
    return (api_params_t *)api_params_slab()->alloc();
}

static inline void
api_params_free (api_params_t *api_params, obj_id_t obj_id, api_op_t api_op)
{
    if (obj_id == api::OBJ_ID_ROUTE_TABLE &&
        (api_op == api::API_OP_CREATE ||
        (api_op == api::API_OP_UPDATE))) {
        if (api_params->route_table_info.routes) {
            SDK_FREE(OCI_MEM_ALLOC_ROUTE_TABLE,
                     api_params->route_table_info.routes);
        }
    }
    return api_params_slab()->free(api_params);
}

}    // namespace api

using api::api_ctxt_t;
using api::api_params_t;

#endif    /** __API_CTXT_HPP__ */
