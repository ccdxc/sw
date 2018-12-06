/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    api_engine.cc
 *
 * @brief   API engine functionality
 */

#include "nic/hal/apollo/framework/api_engine.hpp"

namespace api {

/**< API engine (singleton) instance */
api_engine    g_api_engine

/**
 * @defgroup OCI_API_ENGINE - framework for processing APIs
 * @ingroup OCI_VCN
 * @{
 */

/**
 * @brief    handle batch begin by setting up per API batch context
 */
sdk_ret_t
api_engine::batch_begin(oci_epoch_t epoch) {
    batch_ctxt_.epoch = OCI_EPOCH_INVALID;
    batch_ctxt_.stage = API_BATCH_STAGE_NONE;
    batch_ctxt_.apis.reserve(16);
    return sdk::SDK_RET_OK;
};

/**
 * @brief    commit all the APIs in this batch, release any temporary
 *           state or resources like memory, per API context info etc.
 */
sdk_ret_t
api_engine::batch_commit(void) {

    // clear all batch related info
    batch_ctxt_.apis.clear();
    return sdk::SDK_RET_OK;
}

/**
 * @brief    abort all the APIs in this batch, release any temporary
 *           state or resources like memory, per API context info etc.
 */
sdk_ret_t
api_engine::batch_abort(void) {

    // clear all batch related info
    batch_ctxt_.epoch = OCI_EPOCH_INVALID;
    batch_ctxt_.stage = API_BATCH_STAGE_NONE;
    batch_ctxt_.apis.clear();
    return sdk::SDK_RET_OK;
}

/**
 * @brief    wrapper function for processing all API calls
 */
sdk_ret_t
api_engine::process_api(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_OK;
}

/** @} */    // end of OCI_API_ENGINE

}    // namespace api
