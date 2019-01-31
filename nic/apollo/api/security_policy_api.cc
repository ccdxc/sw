/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    security_policy_api.cc
 *
 * @brief   security policy handling
 */

#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/include/api/oci_security_policy.hpp"

/**
 * @defgroup OCI_SECURITY_POLICY_API - first level of security policy API handling
 * @ingroup OCI_SECURITY_POLICY
 * @{
 */

/**
 * @brief create a security policy
 *
 * @param[in] security_policy security policy information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_security_policy_create (_In_ oci_security_policy_t *security_policy)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = api_params_alloc(api::OBJ_ID_SECURITY_POLICY,
                                           api::API_OP_CREATE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_CREATE;
        api_ctxt.obj_id = api::OBJ_ID_SECURITY_POLICY;
        api_ctxt.api_params->security_policy_info = *security_policy;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/**
 * @brief delete given security policy
 *
 * @param[in] security_policy_key    security policy key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_security_policy_delete (_In_ oci_security_policy_key_t *security_policy_key)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = api_params_alloc(api::OBJ_ID_SECURITY_POLICY,
                                           api::API_OP_DELETE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_DELETE;
        api_ctxt.obj_id = api::OBJ_ID_SECURITY_POLICY;
        api_ctxt.api_params->security_policy_key = *security_policy_key;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/** @} */ // end of OCI_SECURITY_POLICY_API
