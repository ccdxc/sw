/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    tep_api.cc
 *
 * @brief   Tunnel EndPoint (TEP) API handling
 */

#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/tep.hpp"

/**
 * @defgroup OCI_TEP_API - first level of TEP API handling
 * @ingroup OCI_TEP
 * @{
 */

/**
 * @brief create TEP
 *
 * @param[in] tep TEP information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_tep_create (_In_ oci_tep_t *tep)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = api::api_params_alloc(api::OBJ_ID_TEP,
                                                api::API_OP_CREATE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_CREATE;
        api_ctxt.obj_id = api::OBJ_ID_TEP;
        api_ctxt.api_params->tep_info = *tep;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/**
 * @brief delete TEP
 *
 * @param[in] tep_key TEP key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_tep_delete (_In_ oci_tep_key_t *tep_key)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = api::api_params_alloc(api::OBJ_ID_TEP,
                                                api::API_OP_DELETE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_DELETE;
        api_ctxt.obj_id = api::OBJ_ID_TEP;
        api_ctxt.api_params->tep_key = *tep_key;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/** @} */ // end of OCI_TEP_API
