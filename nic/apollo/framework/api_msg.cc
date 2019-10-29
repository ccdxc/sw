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
        // process this batch of APIs synchronously/asynchronously
        ret = api::process_api(bctxt, NULL);
    }
    api_batch_destroy(bctxt);
    return ret;
}

sdk_ret_t
process_api (pds_batch_ctxt_t bctxt, api_ctxt_t *api_ctxt)
{
    bool batched_internally = false;
    sdk::ipc::ipc_msg_ptr rsp;
    pds_batch_params_t batch_params = { 0 };
    api_msg_t *api_msg = (api_msg_t *)bctxt;
    sdk_ret_t ret;

    if (api_msg) {
        if (api_ctxt) {
            // just accumulate this API in the current batch
            api_msg->batch.apis.push_back(api_ctxt);
            return SDK_RET_OK;
        }
    } else {
        if (!api_ctxt) {
            return SDK_RET_INVALID_OP;
        }
        // API call is not part of any batch, create a batch of one
        batch_params.epoch = g_api_epoch_;
        api_msg = (api_msg_t *)api_batch_start(&batch_params);
        if (likely(api_msg != NULL)) {
            api_msg->batch.apis.push_back(api_ctxt);
        }
        batched_internally = true;
    }
    rsp = sdk::ipc::request(core::THREAD_ID_API, API_MSG_ID_BATCH,
                                 &api_msg, sizeof(api_msg));
    ret = *(sdk_ret_t *)rsp->data();
    PDS_TRACE_DEBUG("Rcvd response from API thread, status %u", ret);

    if (batched_internally) {
        // destroy the batch we created
        api_batch_destroy((pds_batch_ctxt_t)api_msg);
    }
    return ret;
}

}    // namespace api
