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
 * @brief    add/update h/w entries, based on the configuration without
 *           activating the epoch
 */
sdk_ret_t
api_engine::update_tables(void) {
    api_ctxt_t    api_ctxt;
    sdk_ret_t     ret;

    /**
     * walk over all the dirty objects and call program_hw() on each object
     */
    for (vector<api_ctxt_t>::iterator it = batch_ctxt_.apis.begin();
         it != batch_ctxt_.apis.end(); ++it) {
        api_ctxt = *it;
        switch (api_ctxt.api_op) {
        case API_OP_CREATE:
            /**
             * call program_hw() callback on the new object, note that the
             * object should have been in the s/w db already by this time but
             * marked as dirty
             */
            ret = api_ctxt.new_obj->program_hw(&api_ctxt);
            SDK_ASSERT_RETURN((ret == sdk::SDK_RET_OK), ret);
            break;

        case API_OP_DELETE:
            /**
             * call cleanup_hw() callback on existing object and mark it for
             * deletion
             * NOTE: delete is same as updating the h/w entries with latest
             *       epoch, which will be activated later in the commit()
             *       stage (by programming tables in stage 0, if needed)
             * TODO: when do we free up this entry from table mgmt. libs ?
             *       we can do delay delete for these (with the caveat that
             *       until then the h/w entries can't be used)
             */
            ret = api_ctxt.curr_obj->cleanup_hw(&api_ctxt);
            SDK_ASSERT_RETURN((ret == sdk::SDK_RET_OK), ret);
            break;

        case API_OP_UPDATE:
            /**
             * call update_hw() callback on the cloned object so new config
             * can be programmed in the h/w everywhere except stage0, which will
             * be programmed during commit() stage later
             * NOTE: during commit() stage, for update case, we will swap new
             *       obj with old/current one in the all the dbs as well before
             *       activating epoch is activated in h/w stage 0 (and old
             *       object should be freed back)
             */
            ret = api_ctxt.new_obj->update_hw(&api_ctxt);
            SDK_ASSERT_RETURN((ret == sdk::SDK_RET_OK), ret);
            break;

        case API_OP_GET:
            break;

        default:
            break;
        }
    }
    return sdk::SDK_RET_OK;
}

/**
 * @brief    any objects having tables in stage 0 of datapath can act in this
 *           epoch activation stage to switch to new epoch
 *           activating the epoch
 */
sdk_ret_t
api_engine::activate_epoch(void) {
    return sdk::SDK_RET_OK;
}

/**
 * @brief    commit all the APIs in this batch, release any temporary
 *           state or resources like memory, per API context info etc.
 */
sdk_ret_t
api_engine::batch_commit(void) {
    sdk_ret_t     ret;

    SDK_ASSERT_RETURN((batch_ctxt_.stage == API_BATCH_STAGE_INIT),
                      sdk::SDK_RET_INVALID_ARG);
    batch_ctxt_.stage = API_BATCH_STAGE_TABLE_UPDATE;
    /**< update all the p4 tables, with the exception of stage 0 */
    ret = update_tables();
    SDK_ASSERT_GOTO((ret == sdk::SDK_RET_OK), error);

    /**
     * advance to next stage of processing and activate the epoch in h/w & s/w
     * by programming stage0 tables, if any.
     * NOTE: this is not expected to fail
     */
    batch_ctxt_.stage = API_BATCH_STAGE_ACTIVATE_EPOCH;
    ret = activate_epoch();
    SDK_ASSERT(ret == sdk::SDK_RET_OK);

    /**< clear all batch related info */
    batch_ctxt_.epoch = OCI_EPOCH_INVALID;
    batch_ctxt_.stage = API_BATCH_STAGE_NONE;
    batch_ctxt_.apis.clear();

    return sdk::SDK_RET_OK;

error:

    batch_ctxt_.stage = API_BATCH_STAGE_ABORT;
    return ret;
}

/**
 * @brief    abort all the APIs in this batch, release any temporary
 *           state or resources like memory, per API context info etc.
 */
sdk_ret_t
api_engine::batch_abort(void) {
    //api_ctxt_t    api_ctxt;
    //sdk_ret_t     ret;

    SDK_ASSERT_RETURN((batch_ctxt_.stage == API_BATCH_STAGE_ABORT),
                      sdk::SDK_RET_INVALID_ARG);

#if 0
    /**
     * walk over all the dirty objects and perform abort operation
     * NOTE: abort is never expected to fail
     */
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
#endif

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
