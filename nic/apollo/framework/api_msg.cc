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

// callback that gets called when process_api is done with it's return code
static void
api_process_sync_result_ (sdk::ipc::ipc_msg_ptr msg, const void *ret)
{
    *(sdk_ret_t *)ret = *(sdk_ret_t *)msg->data();
}

static void
api_process_async_result_ (sdk::ipc::ipc_msg_ptr msg, const void *ctx)
{
    api_msg_t *api_msg = (api_msg_t *)ctx;
    sdk_ret_t ret = *(sdk_ret_t *)msg->data();

    api_msg->batch.response_cb(ret, api_msg->batch.cookie);

    api_batch_destroy((pds_batch_ctxt_t)api_msg);
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
        api_msg->batch.response_cb = batch_params->response_cb;
        api_msg->batch.cookie = batch_params->cookie;
        api_msg->batch.apis.reserve(1024);
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
    // clear the contents of the API vector
    api_msg->batch.apis.clear();
    // free the batch context
    api::api_msg_free(api_msg);
    return SDK_RET_OK;
}

// called directly by client of this library. No async. No batch
sdk_ret_t
process_api (pds_batch_ctxt_t bctxt, api_ctxt_t *api_ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    api_msg_t *api_msg;

    if (bctxt == 0) {
        pds_batch_params_t batch_params = { 0 };
        if (!api_ctxt) {
            return SDK_RET_INVALID_OP;
        }
        // API call is not part of any batch, create a batch of one
        batch_params.epoch = g_api_epoch_;
        api_msg = (api_msg_t *)api_batch_start(&batch_params);
        if (likely(api_msg != NULL)) {
            api_msg->batch.apis.push_back(api_ctxt);
        }
        // send API msg to API thread and receive the response synchronously
        sdk::ipc::request(core::PDS_THREAD_ID_API, API_MSG_ID_BATCH, api_msg,
                          sizeof(*api_msg), api_process_sync_result_, &ret);

        api_batch_destroy((pds_batch_ctxt_t)api_msg);
        return ret;
    }

    api_msg = (api_msg_t *)bctxt;
    if (api_ctxt) {
        // batch commit is not happening yet, just accumulate this API in
        // the current batch
        api_msg->batch.apis.push_back(api_ctxt);
        return SDK_RET_OK;
    }
    // batch commit happening, ship APIs to API thread
    if (api_msg->batch.async) {
        sdk::ipc::request(core::PDS_THREAD_ID_API, API_MSG_ID_BATCH, api_msg,
                          sizeof(*api_msg), api_process_async_result_, api_msg);
        return SDK_RET_OK;
    } else {
        sdk::ipc::request(core::PDS_THREAD_ID_API, API_MSG_ID_BATCH, api_msg,
                          sizeof(*api_msg), api_process_sync_result_, &ret);
        api_batch_destroy((pds_batch_ctxt_t)api_msg);
        return ret;
    }
}

sdk_ret_t
api_batch_commit (pds_batch_ctxt_t bctxt)
{
    api_msg_t *api_msg = (api_msg_t *)bctxt;
    sdk_ret_t ret = SDK_RET_OK;

    // if this is a batch of 0, no need to bother API thread
    if (likely(api_msg->batch.apis.size() > 0)) {
        // process this batch of APIs
        ret = api::process_api(bctxt, NULL);
    } else {
        api_batch_destroy(bctxt);
    }

    return ret;
}

}    // namespace api
