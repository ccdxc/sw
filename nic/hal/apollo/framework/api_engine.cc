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
    api_ctxt_t    api_ctxt;
    sdk_ret_t     ret;

    SDK_ASSERT_RETURN((batch_ctxt_.stage == API_BATCH_STAGE_INIT),
                      sdk::SDK_RET_INVALID_ARG);
    batch_ctxt_.stage = API_BATCH_STAGE_COMMIT;

    // walk over all the dirty objects and perform commit operation
    // NOTE: we don't expect any operation to fail during commit phase but hash
    // tables are a bit tricky as there is no mechanism to pre-reserve entries
    // for them yet, so to be generic we do support failure of commit operation
    for (vector<api_ctxt_t>::iterator it = batch_ctxt_.apis.begin();
         it != batch_ctxt_.apis.end(); ++it) {
        api_ctxt = *it;
        if (api_ctxt.curr_obj) {
            // DELETE/UPDATE case
            ret = api_ctxt.curr_obj->commit(&api_ctxt);
            if (ret != sdk::SDK_RET_OK) {
                batch_ctxt_.stage = API_BATCH_STAGE_ABORT;
                return ret;
            }
        } else {
            // CREATE case
            ret = api_ctxt.new_obj->commit(&api_ctxt);
            if (ret != sdk::SDK_RET_OK) {
                batch_ctxt_.stage = API_BATCH_STAGE_ABORT;
                return ret;
            }
        }
    }

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
    api_ctxt_t    api_ctxt;
    sdk_ret_t     ret;

    SDK_ASSERT_RETURN((batch_ctxt_.stage == API_BATCH_STAGE_ABORT),
                      sdk::SDK_RET_INVALID_ARG);

    // walk over all the dirty objects and perform abort operation
    // NOTE: abort is never expected to fail
    for (vector<api_ctxt_t>::iterator it = batch_ctxt_.apis.begin();
         it != batch_ctxt_.apis.end(); ++it) {
        api_ctxt = *it;
        if (api_ctxt.curr_obj) {
            // DELETE/UPDATE case
            ret = api_ctxt.curr_obj->abort(&api_ctxt);
            if (ret != sdk::SDK_RET_OK) {
                // TODO: log it, but continue
            }
        } else {
            // CREATE case
            ret = api_ctxt.new_obj->abort(&api_ctxt);
            if (ret != sdk::SDK_RET_OK) {
                // TODO: log it, but continue
            }
        }
    }
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
    sdk_ret_t    ret = sdk::SDK_RET_OK;

    SDK_ASSERT_RETURN((batch_ctxt_.stage == API_BATCH_STAGE_INIT),
                      sdk::SDK_RET_INVALID_OP);
    switch (api_ctxt->api_op) {
    case API_OP_CREATE:
        /**< instantiate a new object */
        api_ctxt->curr_obj = NULL;
        api_ctxt->new_obj = api_base::factory(api_ctxt);
        if (unlikely(api_ctxt->new_obj == NULL)) {
            batch_ctxt_.stage = API_BATCH_STAGE_ABORT;
            return sdk::SDK_RET_ERR;
        }

        /**< add API context to current batch list so we can access it during
         * commit/abort phase
         */
        batch_ctxt_.apis.push_back(*api_ctxt);

        /**< initialize the object, allocate s/w, h/w resources needed for it */
        ret = api_ctxt->new_obj->process_create(api_ctxt);
        if (ret != sdk::SDK_RET_OK) {
            goto error;
        }

        /**< mark the object as dirty to indicate that an operation is in
         * progress and add the dirty obj to the db
         */
        api_ctxt->new_obj->set_dirty();
        api_ctxt->new_obj->add_to_db();
        break;

    case API_OP_DELETE:
        api_ctxt->new_obj = NULL;
        /**< find non-dirty object since we don't support add and delete in same
         *   batch as it doesn't make sense (may be it does from external
         *   controller point of view, if its batching periodically
         */
        api_ctxt->curr_obj = api_base::find_obj(api_ctxt, true);
        if (api_ctxt->curr_obj) {
            batch_ctxt_.apis.push_back(*api_ctxt);
            ret = api_ctxt->new_obj->process_delete(api_ctxt);
            if (ret != sdk::SDK_RET_OK) {
                goto error;
            }
        } else {
            /**< not expected to happen, but ignore the error by not adding the
             * api ctxt for later commit/abort processing
             */
        }
        break;

    case API_OP_UPDATE:
        api_ctxt->curr_obj = api_base::find_obj(api_ctxt, false);
        if (api_ctxt->curr_obj) {
            batch_ctxt_.apis.push_back(*api_ctxt);
            /**< clone current obj and modify the cloned obj, during commit
             * phase we switch cloned obj with current obj and free the current
             * obj
             */
            api_ctxt->new_obj = api_ctxt->curr_obj->clone();
            api_ctxt->new_obj->process_update(api_ctxt);
        } else {
            /**< not expected to happen, but ignore the error by not adding the
             * api ctxt for later commit/abort processing
             */
        }
        break;

    case API_OP_GET:
        break;

    default:
        break;
    }

    return sdk::SDK_RET_OK;

error:

    batch_ctxt_.stage = API_BATCH_STAGE_ABORT;
    return ret;
}

/** @} */    // end of OCI_API_ENGINE

}    // namespace api
