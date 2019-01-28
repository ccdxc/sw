/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    mapping_api.cc
 *
 * @brief   mapping API handling
 */

#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/mapping.hpp"

/**
 * @defgroup OCI_MAPPING_API - first level of mapping API handling
 * @ingroup OCI_MAPPING
 * @{
 */

/**
 * @brief create mapping
 *
 * @param[in] mapping mapping information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_mapping_create (_In_ oci_mapping_t *mapping)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = api::api_params_alloc(api::OBJ_ID_MAPPING,
                                                api::API_OP_CREATE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_CREATE;
        api_ctxt.obj_id = api::OBJ_ID_MAPPING;
        api_ctxt.api_params->mapping_info = *mapping;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/**
 * @brief delete mapping
 *
 * @param[in] mapping_key mapping key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_mapping_delete (_In_ oci_mapping_key_t *mapping_key)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = api::api_params_alloc(api::OBJ_ID_MAPPING,
                                                api::API_OP_DELETE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_DELETE;
        api_ctxt.obj_id = api::OBJ_ID_MAPPING;
        api_ctxt.api_params->mapping_key = *mapping_key;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/** @} */    // end of OCI_MAPPING_API
