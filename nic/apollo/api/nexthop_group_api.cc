//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements nexthop group CRUD APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/nexthop.hpp"
#include "nic/apollo/api/nexthop_group_state.hpp"

static sdk_ret_t
pds_nexthop_group_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                              pds_obj_key_t *key,
                              pds_nexthop_group_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_NEXTHOP_GROUP, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->nexthop_group_key = *key;
        } else {
            api_ctxt->api_params->nexthop_group_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline sdk_ret_t
pds_nexthop_group_stats_fill (pds_nexthop_group_stats_t *stats,
                              nexthop_group *entry)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_nexthop_group_status_fill (pds_nexthop_group_status_t *status,
                               nexthop_group *entry)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_nexthop_group_spec_fill (pds_nexthop_group_spec_t *spec,
                             nexthop_group *entry)
{
    return SDK_RET_OK;
}

static inline nexthop_group *
pds_nexthop_group_find (pds_obj_key_t *key)
{
    return (nexthop_group_db()->find(key));
}

//----------------------------------------------------------------------------
// nexthop API entry point implementation
//----------------------------------------------------------------------------

sdk_ret_t
pds_nexthop_group_create (_In_ pds_nexthop_group_spec_t *spec,
                          _In_ pds_batch_ctxt_t bctxt)
{
    return pds_nexthop_group_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_nexthop_group_read (_In_ pds_obj_key_t *key,
                        _Out_ pds_nexthop_group_info_t *info)
{
    nexthop_group *entry;

    if (key == NULL || info == NULL) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_nexthop_group_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    return entry->read(info);
}

sdk_ret_t
pds_nexthop_group_update (_In_ pds_nexthop_group_spec_t *spec,
                          _In_ pds_batch_ctxt_t bctxt)
{
    return pds_nexthop_group_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_nexthop_group_delete (_In_ pds_obj_key_t *key,
                          _In_ pds_batch_ctxt_t bctxt)
{
    return pds_nexthop_group_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
