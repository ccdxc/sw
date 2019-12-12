//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// API message processing helper functions
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/apollo/core/core.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/api/include/pds.hpp"

namespace api {

static inline void process_api_internal_(pds_batch_ctxt_t bctxt,
                                         api_ctxt_t *api_ctxt,
                                         process_result_cb result_cb,
                                         const void *result_cb_ctx);

static pds_epoch_t    g_api_epoch_ = 0xDEADFEED;

/// \brief    wrapper function to allocate an API msg
/// \return   pointer to allocated API msg or NULL
static inline api_msg_t *
api_msg_alloc (void)
{
    return (api_msg_t *)api_msg_slab()->alloc();
}

/// \brief    wrapper function to free an api msg
/// \param[in] msg    API msg to be freed
static inline void
api_msg_free (api_msg_t *msg)
{
    api_msg_slab()->free(msg);
}

pds_batch_ctxt_t
api_batch_start (pds_batch_params_t *batch_params)
{
    api_msg_t *api_msg;

    if (unlikely((batch_params == NULL) ||
                 (batch_params->epoch == PDS_EPOCH_INVALID))) {
        return PDS_BATCH_CTXT_INVALID;
    }

    // allocate IPC msg for this batch & initialize the context
    api_msg = api::api_msg_alloc();
    if (likely(api_msg)) {
        api_msg->msg_id = api::API_MSG_ID_BATCH;
        api_msg->batch.epoch = batch_params->epoch;
        api_msg->batch.async = batch_params->async;
        api_msg->batch.cookie = batch_params->cookie;
        api_msg->batch.apis.reserve(64);
        api_msg->batch.batched_internally = false;
        api_msg->batch.result_cb = NULL;
        api_msg->batch.result_cb_ctx = NULL;
    }
    return (pds_batch_ctxt_t)api_msg;
}

sdk_ret_t
api_batch_destroy (pds_batch_ctxt_t bctxt)
{
    api_msg_t *api_msg = (api_msg_t *)bctxt;

    // free all the API contexts
    for (auto it = api_msg->batch.apis.begin();
         it != api_msg->batch.apis.end(); ++it) {
        api::api_ctxt_free(*it);
    }
    // free the batch context
    api::api_msg_free(api_msg);
    return SDK_RET_OK;
}

sdk_ret_t
api_batch_commit (pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    api_msg_t *api_msg = (api_msg_t *)bctxt;

    // if this is a batch of 0, no need to bother API thread
    if (likely(api_msg->batch.apis.size() > 0)) {
        // process this batch of APIs synchronously
        ret = api::process_api(bctxt, NULL);
    }
    api_batch_destroy(bctxt);
    return ret;
}

// this callback is invoked after getting response from the API thread,
// to free all the API msg related state allocated for the batch processing of
// APIs by API thread
static void
api_msg_response_cb (sdk::ipc::ipc_msg_ptr msg, const void *cookie,
                    const void *ctx)
{
    api_msg_t *api_msg = (api_msg_t *)cookie;

    if (api_msg->batch.result_cb) {
        api_msg->batch.result_cb(*(sdk_ret_t *)msg->data(),
                                 api_msg->batch.result_cb_ctx);
    }

    if (api_msg->batch.batched_internally) {
        api_batch_destroy((pds_batch_ctxt_t)api_msg);
    }
}

// this is a callback that gets called when process_api is done with it's return
// code
static void
process_result_ (sdk_ret_t ret, const void *ctx)
{
    *(sdk_ret_t *)ctx = ret;
}

static inline void
process_api_internal_ (pds_batch_ctxt_t bctxt, api_ctxt_t *api_ctxt,
                       process_result_cb result_cb, const void *result_cb_ctx)
{
    pds_batch_params_t batch_params = { 0 };
    api_msg_t *api_msg = (api_msg_t *)bctxt;

    sdk::ipc::reg_response_handler(API_MSG_ID_BATCH, api_msg_response_cb,
                                   NULL);

    if (api_msg) {
        if (api_ctxt) {
            // just accumulate this API in the current batch
            api_msg->batch.apis.push_back(api_ctxt);
            result_cb(SDK_RET_OK, result_cb_ctx);
            return;
        }
    } else {
        if (!api_ctxt) {
            result_cb(SDK_RET_INVALID_OP, result_cb_ctx);
            return;
        }
        // API call is not part of any batch, create a batch of one
        batch_params.epoch = g_api_epoch_;
        api_msg = (api_msg_t *)api_batch_start(&batch_params);
        if (likely(api_msg != NULL)) {
            api_msg->batch.apis.push_back(api_ctxt);
        }
        api_msg->batch.batched_internally = true;
    }

    api_msg->batch.result_cb = result_cb;
    api_msg->batch.result_cb_ctx = result_cb_ctx;
    sdk::ipc::request(core::THREAD_ID_API, API_MSG_ID_BATCH, &api_msg,
                      sizeof(api_msg), (void *)api_msg);
}

sdk_ret_t
process_api (pds_batch_ctxt_t bctxt, api_ctxt_t *api_ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    
    process_api_internal_(bctxt, api_ctxt, process_result_, &ret);

    return ret;
}

}    // namespace api
