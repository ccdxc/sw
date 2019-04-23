/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    policy_api.cc
 *
 * @brief   policy API handling
 */

#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/policy.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/policy_state.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"

/**
 * @defgroup PDS_POLICY_API - first level of security policy API handling
 * @ingroup PDS_POLICY
 * @{
 */
static sdk_ret_t
pds_policy_api_handle (api::api_op_t op, pds_policy_key_t *key,
                       pds_policy_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != sdk::SDK_RET_OK)
        return rv;

    api_ctxt.api_params = api::api_params_alloc(api::OBJ_ID_POLICY, op);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = op;
        api_ctxt.obj_id = api::OBJ_ID_POLICY;
        if (op == api::API_OP_DELETE)
            api_ctxt.api_params->policy_key = *key;
        else
            api_ctxt.api_params->policy_spec = *spec;
        return (api::g_api_engine.process_api(&api_ctxt));
    }
    return sdk::SDK_RET_OOM;
}

static inline sdk_ret_t
pds_policy_spec_fill (policy *entry, pds_policy_spec_t *spec)
{
    memcpy(&spec->key,
           (pds_policy_key_t *)policy::policy_key_func_get((void *)entry),
           sizeof(pds_policy_key_t));
    spec->af = entry->af();
    spec->direction = entry->dir();
    return SDK_RET_OK;
}

static inline policy *
pds_policy_entry_find (pds_policy_key_t *key)
{
    return policy_db()->policy_find(key);
}

//----------------------------------------------------------------------------
// Policy API entry point implementation
//----------------------------------------------------------------------------

/**
 * @brief create a security policy
 *
 * @param[in] spec security policy spec
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_policy_create (pds_policy_spec_t *spec)
{
    return pds_policy_api_handle(api::API_OP_CREATE, NULL, spec);
}

/**
 * @brief delete given security policy
 *
 * @param[in] key    security policy key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_policy_delete (pds_policy_key_t *key)
{
    return pds_policy_api_handle(api::API_OP_DELETE, key, NULL);
}

sdk_ret_t
pds_policy_read(pds_policy_key_t *key, pds_policy_info_t *info)
{
    sdk::sdk_ret_t rv;
    policy *entry = NULL;

    if (key == NULL || info == NULL)
        return sdk::SDK_RET_INVALID_ARG;

    if ((entry = pds_policy_entry_find(key)) == NULL)
        return sdk::SDK_RET_ENTRY_NOT_FOUND;

    if ((rv = pds_policy_spec_fill(entry, &info->spec)) != sdk::SDK_RET_OK)
        return rv;

    return SDK_RET_OK;
}

/** @} */ // end of PDS_POLICY_API
