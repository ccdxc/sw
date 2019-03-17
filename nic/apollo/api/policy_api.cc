/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    policy_api.cc
 *
 * @brief   policy API handling
 */

#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"

/**
 * @defgroup PDS_POLICY_API - first level of security policy API handling
 * @ingroup PDS_POLICY
 * @{
 */

/**
 * @brief create a security policy
 *
 * @param[in] spec security policy spec
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_policy_create (pds_policy_spec_t *spec)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = api_params_alloc(api::OBJ_ID_POLICY,
                                           api::API_OP_CREATE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_CREATE;
        api_ctxt.obj_id = api::OBJ_ID_POLICY;
        api_ctxt.api_params->policy_spec = *spec;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
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
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = api_params_alloc(api::OBJ_ID_POLICY,
                                           api::API_OP_DELETE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_DELETE;
        api_ctxt.obj_id = api::OBJ_ID_POLICY;
        api_ctxt.api_params->policy_key = *key;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/** @} */ // end of PDS_POLICY_API
