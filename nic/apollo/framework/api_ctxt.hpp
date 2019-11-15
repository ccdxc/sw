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
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds_mirror.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/include/pds_meter.hpp"
#include "nic/apollo/api/include/pds_tag.hpp"
#include "nic/apollo/api/include/pds_service.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/api/include/pds_policer.hpp"

namespace api {

/// \brief API specific parameters
typedef union api_params_u {
    pds_batch_params_t           batch_spec;
    pds_device_spec_t            device_spec;
    pds_if_key_t                 if_key;
    pds_if_spec_t                if_spec;
    pds_tep_key_t                tep_key;
    pds_tep_spec_t               tep_spec;
    pds_vpc_key_t                vpc_key;
    pds_vpc_spec_t               vpc_spec;
    pds_subnet_key_t             subnet_key;
    pds_subnet_spec_t            subnet_spec;
    pds_vnic_key_t               vnic_key;
    pds_vnic_spec_t              vnic_spec;
    pds_mapping_key_t            mapping_key;
    pds_mapping_spec_t           mapping_spec;
    pds_route_table_key_t        route_table_key;
    pds_route_table_spec_t       route_table_spec;
    pds_policy_key_t             policy_key;
    pds_policy_spec_t            policy_spec;
    pds_mirror_session_key_t     mirror_session_key;
    pds_mirror_session_spec_t    mirror_session_spec;
    pds_meter_key_t              meter_key;
    pds_meter_spec_t             meter_spec;
    pds_tag_key_t                tag_key;
    pds_tag_spec_t               tag_spec;
    pds_svc_mapping_key_t        svc_mapping_key;
    pds_svc_mapping_spec_t       svc_mapping_spec;
    pds_vpc_peer_key_t           vpc_peer_key;
    pds_vpc_peer_spec_t          vpc_peer_spec;
    pds_nexthop_key_t            nexthop_key;
    pds_nexthop_spec_t           nexthop_spec;
    pds_nexthop_group_key_t      nexthop_group_key;
    pds_nexthop_group_spec_t     nexthop_group_spec;
    pds_policer_spec_t           policer_spec;
} api_params_t;

/// \brief Per API context maintained by framework while processing
typedef struct api_ctxt_s {
    api_op_t      api_op;        ///< api operation
    obj_id_t      obj_id;        ///< object identifier
    api_params_t  *api_params;   ///< API specific params
} api_ctxt_t;

slab *api_params_slab(void);
// TODO: we can get rid of this and use api_msg_slab directly
slab *api_ctxt_slab(void);

static inline api_ctxt_t *
api_ctxt_alloc (obj_id_t obj_id, api_op_t api_op)
{
    api_ctxt_t *api_ctxt;

    api_ctxt = (api_ctxt_t *)api_ctxt_slab()->alloc();
    if (api_ctxt) {
        api_ctxt->obj_id = obj_id;
        api_ctxt->api_op = api_op;
        api_ctxt->api_params = (api_params_t *)api_params_slab()->alloc();
        if (unlikely(api_ctxt->api_params == NULL)) {
            api_ctxt_slab()->free(api_ctxt);
            return NULL;
        }
    }
    return api_ctxt;
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
                api_params->route_table_spec.routes = NULL;
            }
        }
        break;

    case api::OBJ_ID_POLICY:
        if ((api_op == api::API_OP_CREATE) || (api_op == api::API_OP_UPDATE)) {
            if (api_params->policy_spec.rules) {
                SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY,
                         api_params->policy_spec.rules);
                api_params->policy_spec.rules = NULL;
            }
        }
        break;

    case api::OBJ_ID_METER:
        if ((api_op == api::API_OP_CREATE) || (api_op == api::API_OP_UPDATE)) {
            if (api_params->meter_spec.rules) {
                for (uint32_t i = 0; i < api_params->meter_spec.num_rules;
                     i++) {
                    SDK_FREE(PDS_MEM_ALLOC_ID_METER,
                             api_params->meter_spec.rules[i].prefixes);
                }
                SDK_FREE(PDS_MEM_ALLOC_METER,
                         api_params->meter_spec.rules);
                api_params->meter_spec.rules = NULL;
            }
        }
        break;

    case api::OBJ_ID_TAG:
        if ((api_op == api::API_OP_CREATE) || (api_op == api::API_OP_UPDATE)) {
            if (api_params->tag_spec.rules) {
                for (uint32_t i = 0; i < api_params->tag_spec.num_rules; i++) {
                    SDK_FREE(PDS_MEM_ALLOC_ID_TAG,
                             api_params->tag_spec.rules[i].prefixes);
                }
                SDK_FREE(PDS_MEM_ALLOC_TAG, api_params->tag_spec.rules);
                api_params->tag_spec.rules = NULL;
            }
        }
        break;

    default:
        break;
    }
    api_params_slab()->free(api_params);
}

static inline void
api_ctxt_free (api_ctxt_t *api_ctxt)
{
    if (api_ctxt->api_params) {
        api_params_free(api_ctxt->api_params,
                        api_ctxt->obj_id, api_ctxt->api_op);
    }
    api_ctxt_slab()->free(api_ctxt);
}

}    // namespace api

using api::api_ctxt_t;
using api::api_params_t;

#endif    // __FRAMEWORK_API_CTXT_HPP__
