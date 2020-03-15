//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// API params specific APIs
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api.h"
#include "nic/apollo/framework/api_params.hpp"

namespace api {

static slab *g_api_params_slab_ = NULL;

void
api_params_free (api_params_t *api_params, obj_id_t obj_id, api_op_t api_op)
{
    switch (obj_id) {
    case OBJ_ID_ROUTE_TABLE:
        if ((api_op == API_OP_CREATE) || (api_op == API_OP_UPDATE)) {
            if (api_params->route_table_spec.route_info) {
                SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                         api_params->route_table_spec.route_info);
                api_params->route_table_spec.route_info = NULL;
            }
        }
        break;

    case OBJ_ID_POLICY:
        if ((api_op == API_OP_CREATE) || (api_op == API_OP_UPDATE)) {
            if (api_params->policy_spec.rule_info) {
                SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY,
                         api_params->policy_spec.rule_info);
                api_params->policy_spec.rule_info = NULL;
            }
        }
        break;

    case OBJ_ID_METER:
        if ((api_op == API_OP_CREATE) || (api_op == API_OP_UPDATE)) {
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

    case OBJ_ID_TAG:
        if ((api_op == API_OP_CREATE) || (api_op == API_OP_UPDATE)) {
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

slab *
api_params_slab (void)
{
    return g_api_params_slab_;
}

sdk_ret_t
api_params_init (void)
{
    g_api_params_slab_ =
        slab::factory("api-params", PDS_SLAB_ID_API_PARAMS,
                      sizeof(api_params_t), 512, true, true, true, NULL);
    return SDK_RET_OK;
}

}    // namespace
