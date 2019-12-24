//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements security policy CRUD APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/policy.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/policy_state.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"

static sdk_ret_t
pds_policy_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                       pds_policy_key_t *key, pds_policy_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_POLICY, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->policy_key = *key;
        } else {
            api_ctxt->api_params->policy_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline policy *
pds_policy_entry_find (pds_policy_key_t *key)
{
    return policy_db()->find(key);
}

//----------------------------------------------------------------------------
// Policy API entry point implementation
//----------------------------------------------------------------------------

sdk_ret_t
pds_policy_create (_In_ pds_policy_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_policy_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_policy_read (_In_ pds_policy_key_t *key, _Out_ pds_policy_info_t *info)
{
    policy *entry;

    if (key == NULL || info == NULL)
        return SDK_RET_INVALID_ARG;

    if ((entry = pds_policy_entry_find(key)) == NULL)
        return SDK_RET_ENTRY_NOT_FOUND;

    return entry->read(info);
}

sdk_ret_t
pds_policy_update (_In_ pds_policy_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_policy_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_policy_delete (_In_ pds_policy_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_policy_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
