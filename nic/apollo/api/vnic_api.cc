/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    vnic_api.cc
 *
 * @brief   vnic API handling
 */

#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/vnic.hpp"

/**
 * @defgroup OCI_VNIC_API - first level of vnic API handling
 * @ingroup OCI_VNIC
 * @{
 */

/**
 * @brief create vnic
 *
 * @param[in] vnic vnic information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_vnic_create (_In_ oci_vnic_t *vnic)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = api::api_params_alloc(api::OBJ_ID_VNIC,
                                                api::API_OP_CREATE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_CREATE;
        api_ctxt.obj_id = api::OBJ_ID_VNIC;
        api_ctxt.api_params->vnic_info = *vnic;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/**
 * @brief delete vnic
 *
 * @param[in] vnic_key vnic key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_vnic_delete (_In_ oci_vnic_key_t *vnic_key)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = api::api_params_alloc(api::OBJ_ID_VNIC,
                                                api::API_OP_DELETE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_DELETE;
        api_ctxt.obj_id = api::OBJ_ID_VNIC;
        api_ctxt.api_params->vnic_key = *vnic_key;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/** @} */ // end of OCI_VNIC_API
