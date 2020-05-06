//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements DHCP related CRUD APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/pds_state.hpp"

static sdk_ret_t
pds_dhcp_policy_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                            pds_obj_key_t *key,
                            pds_dhcp_policy_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    // allocate API context
    api_ctxt = api::api_ctxt_alloc(OBJ_ID_DHCP_POLICY, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->key = *key;
        } else {
            api_ctxt->api_params->dhcp_policy_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline dhcp_policy *
pds_dhcp_policy_find (pds_obj_key_t *key)
{
    return (dhcp_db()->find(key));
}

//----------------------------------------------------------------------------
// DHCP policy APIs entry point implementation
//----------------------------------------------------------------------------

sdk_ret_t
pds_dhcp_policy_create (_In_ pds_dhcp_policy_spec_t *spec,
                        _In_ pds_batch_ctxt_t bctxt)
{
    return pds_dhcp_policy_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_dhcp_policy_read (_In_ pds_obj_key_t *key,
                      _Out_ pds_dhcp_policy_info_t *info)
{
    dhcp_policy *policy;

    if (key == NULL || info == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    if ((policy = pds_dhcp_policy_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    return policy->read(info);
}

typedef struct pds_dhcp_policy_read_args_s {
    dhcp_policy_read_cb_t cb;
    void *ctxt;
} pds_dhcp_policy_read_args_t;

bool
pds_dhcp_policy_info_from_entry (void *entry, void *ctxt)
{
    dhcp_policy *policy = (dhcp_policy *)entry;
    pds_dhcp_policy_read_args_t *args = (pds_dhcp_policy_read_args_t *)ctxt;
    pds_dhcp_policy_info_t info;

    memset(&info, 0, sizeof(pds_dhcp_policy_info_t));

    // call entry read
    policy->read(&info);

    // call cb on info
    args->cb(&info, args->ctxt);

    return false;
}

sdk_ret_t
pds_dhcp_policy_read_all (dhcp_policy_read_cb_t cb, void *ctxt)
{
    pds_dhcp_policy_read_args_t args = { 0 };

    args.ctxt = ctxt;
    args.cb = cb;

    return dhcp_db()->walk(pds_dhcp_policy_info_from_entry, &args);
}

sdk_ret_t
pds_dhcp_policy_update (_In_ pds_dhcp_policy_spec_t *spec,
                        _In_ pds_batch_ctxt_t bctxt)
{
    return pds_dhcp_policy_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_dhcp_policy_delete (_In_ pds_obj_key_t *key,
                        _In_ pds_batch_ctxt_t bctxt)
{
    return pds_dhcp_policy_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
