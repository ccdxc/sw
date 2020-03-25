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
    if (key == NULL || info == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
pds_dhcp_policy_read_all (dhcp_policy_read_cb_t cb, void *ctxt)
{
    return SDK_RET_INVALID_OP;
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
