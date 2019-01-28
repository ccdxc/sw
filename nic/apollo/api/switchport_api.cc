/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    switchport_api.cc
 *
 * @brief   switchport API handling
 */

#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/switchport.hpp"

/**
 * @defgroup OCI_SWITCHPORT_API - first level of switchport API handling
 * @ingroup OCI_SWITCHPORT
 * @{
 */

/**
 * @brief create switchport
 *
 * @param[in] switchport switchport information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_switchport_create (_In_ oci_switchport_t *switchport)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = api::api_params_alloc(api::OBJ_ID_SWITCHPORT,
                                                api::API_OP_CREATE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_CREATE;
        api_ctxt.obj_id = api::OBJ_ID_SWITCHPORT;
        api_ctxt.api_params->switchport_info = *switchport;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/** @} */    // end of OCI_SWITCHPORT_API
