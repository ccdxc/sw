//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file deals with internal API context information
///
//----------------------------------------------------------------------------

#ifndef __FRAMEWORK_API_CTXT_HPP__
#define __FRAMEWORK_API_CTXT_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/api/mapping.hpp"
#include "nic/apollo/api/include/pds_vcn.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"

namespace api {

/// \brief API specific parameters
typedef union api_params_u {
    pds_device_spec_t         device_spec;
    pds_tep_key_t             tep_key;
    pds_tep_spec_t            tep_spec;
    pds_vcn_key_t             vcn_key;
    pds_vcn_spec_t            vcn_spec;
    pds_subnet_key_t          subnet_key;
    pds_subnet_spec_t         subnet_spec;
    pds_vnic_key_t            vnic_key;
    pds_vnic_spec_t           vnic_spec;
    pds_mapping_key_t         mapping_key;
    pds_mapping_spec_t        mapping_spec;
    pds_route_table_key_t     route_table_key;
    pds_route_table_spec_t    route_table_spec;
    pds_policy_key_t          policy_key;
    pds_policy_spec_t         policy_spec;
} api_params_t;

/// \brief Per API context maintained by framework while processing
typedef struct api_ctxt_s {
    api_op_t      api_op;        ///< API operation
    obj_id_t      obj_id;        ///< Object identifier
    api_params_t *api_params;    ///< API specific params
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
    switch (obj_id) {
    case api::OBJ_ID_ROUTE_TABLE:
        if ((api_op == api::API_OP_CREATE) || (api_op == api::API_OP_UPDATE)) {
            if (api_params->route_table_spec.routes) {
                SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                         api_params->route_table_spec.routes);
            }
        }
        break;

    case api::OBJ_ID_POLICY:
        if ((api_op == api::API_OP_CREATE) || (api_op == api::API_OP_UPDATE)) {
            if (api_params->policy_spec.rules) {
                SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY,
                         api_params->policy_spec.rules);
            }
        }
        break;

    default:
        break;
    }
    return api_params_slab()->free(api_params);
}

}    // namespace api

using api::api_ctxt_t;
using api::api_params_t;

#endif    // __FRAMEWORK_API_CTXT_HPP__
