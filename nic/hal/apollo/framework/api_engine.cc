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
api_engine    g_api_engine;

/**
 * @defgroup OCI_API_ENGINE - framework for processing APIs
 * @ingroup OCI_VCN
 * @{
 */

/**
 * @brief    handle batch begin by setting up per API batch context
 */
sdk_ret_t
api_engine::batch_begin(oci_batch_params_t *params) {
    SDK_ASSERT_RETURN((params != NULL), sdk::SDK_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((params->epoch != OCI_EPOCH_INVALID),
                      sdk::SDK_RET_INVALID_ARG);
    batch_ctxt_.epoch = params->epoch;
    batch_ctxt_.stage = API_BATCH_STAGE_INIT;
    batch_ctxt_.apis.reserve(16);
    return sdk::SDK_RET_OK;
};

/**
 * @brief    commit all the APIs in this batch, release any temporary
 *           state or resources like memory, per API context info etc.
 */
sdk_ret_t
api_engine::batch_commit(void) {
    SDK_ASSERT_RETURN((batch_ctxt_.stage == API_BATCH_STAGE_INIT),
                      sdk::SDK_RET_INVALID_ARG);
    batch_ctxt_.stage = API_BATCH_STAGE_COMMIT;

    // clear all batch related info
    batch_ctxt_.epoch = OCI_EPOCH_INVALID;
    batch_ctxt_.stage = API_BATCH_STAGE_NONE;
    batch_ctxt_.apis.clear();
    return sdk::SDK_RET_OK;
}

/**
 * @brief    abort all the APIs in this batch, release any temporary
 *           state or resources like memory, per API context info etc.
 */
sdk_ret_t
api_engine::batch_abort(void) {
    SDK_ASSERT_RETURN((batch_ctxt_.stage == API_BATCH_STAGE_INIT),
                      sdk::SDK_RET_INVALID_ARG);
    batch_ctxt_.stage = API_BATCH_STAGE_ABORT;

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
    SDK_ASSERT_RETURN((batch_ctxt_.stage == API_BATCH_STAGE_INIT),
                      sdk::SDK_RET_INVALID_OP);
    switch (api_ctxt->api_op) {
    case API_OP_CREATE:
        /** instantiate, initialize a new object and allocate s/w & h/w
         * resources needed for it
         */
        api_ctxt->curr_obj = NULL;
        api_ctxt->new_obj = api_base::factory(api_ctxt);
        if (unlikely(api_ctxt->new_obj == NULL)) {
            batch_ctxt_.stage = API_BATCH_STAGE_ABORT;
            return sdk::SDK_RET_ERR;
        }

        /**< mark the object as dirty (to indicate that add/modify is in
         *   progress and add the dirty obj to the db
         */
        api_ctxt->new_obj->set_dirty();
        api_ctxt->new_obj->add_to_db();

        /**< add the API context to the current batch list so we can access
         *   it during the commit/abort phase
         */
        batch_ctxt_.apis.push_back(*api_ctxt);
        break;

    case API_OP_DELETE:
        api_ctxt->new_obj = NULL;
        /**< find non-dirty object since we don't support add and delete in same
         *   batch as it doesn't make sense (may be it does from external
         *   controller point of view, if its batching periodically
         */
        api_ctxt->curr_obj = api_base::find_obj(api_ctxt, true);
        SDK_ASSERT(api_ctxt->curr_obj != NULL);
        batch_ctxt_.apis.push_back(*api_ctxt);
        break;

    case API_OP_UPDATE:
#if 0
        api_ctxt->curr_obj = api_base::find_obj(api_ctxt, false);
        SDK_ASSERT(api_ctxt->curr_obj != NULL);
        api_ctxt->new_obj = api_ctxt->curr_obj->clone();
        api_ctxt->new_obj->update(api_ctxt);
#endif
        break;

    case API_OP_GET:
        break;

    default:
        break;
    }
    return sdk::SDK_RET_OK;
}

/** @} */    // end of OCI_API_ENGINE

}    // namespace api
