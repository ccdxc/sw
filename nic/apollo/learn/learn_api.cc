//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// learn thread's API processing interface
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/core.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/learn/ep_utils.hpp"
#include "nic/apollo/learn/learn.hpp"
#include "nic/apollo/learn/learn_api.hpp"
#include "nic/apollo/learn/learn_ctxt.hpp"
#include "nic/apollo/learn/learn_thread.hpp"

namespace learn {

static void
process_async_result (sdk::ipc::ipc_msg_ptr msg, const void *ctx)
{
    api::api_msg_t *api_msg = (api_msg_t *)ctx;
    sdk_ret_t ret = *(sdk_ret_t *)msg->data();

    api_msg->batch.response_cb(ret, api_msg->batch.cookie);
    api::api_batch_destroy((pds_batch_ctxt_t)api_msg);
}

static void
process_sync_result (sdk::ipc::ipc_msg_ptr msg, const void *ret)
{
    *(sdk_ret_t *)ret = *(sdk_ret_t *)msg->data();
}

sdk_ret_t
api_batch_commit (pds_batch_ctxt_t bctxt)
{
    api::api_msg_t *api_msg = (api::api_msg_t *)bctxt;
    sdk_ret_t ret;

    // if learning is disabled, directly commit to API thread
    if (!learning_enabled()) {
        PDS_TRACE_DEBUG("Learning is disabled, committing API batch directly");
        return pds_batch_commit(bctxt);
    }

    if (likely(api_msg->batch.apis.size() > 0)) {
        if (api_msg->batch.async) {
            sdk::ipc::request(core::PDS_THREAD_ID_LEARN, LEARN_MSG_ID_API,
                              api_msg, sizeof(*api_msg), process_async_result,
                              api_msg);
            return SDK_RET_OK;
        } else {
            sdk::ipc::request(core::PDS_THREAD_ID_LEARN, LEARN_MSG_ID_API,
                              api_msg, sizeof(*api_msg), process_sync_result,
                              &ret);
            api::api_batch_destroy(bctxt);
            return ret;
        }
    } else {
        pds_batch_destroy(bctxt);
    }
    return SDK_RET_OK;
}

static inline bool
passthrough_api_batch (api::api_msg_t *api_msg)
{
    api_ctxt_t *api_ctxt = *(api_msg->batch.apis.begin());

    SDK_ASSERT(api_ctxt);
    return (api_ctxt->obj_id != OBJ_ID_MAPPING);
}

static sdk_ret_t
process_passthrough_api_batch (api::api_msg_t *api_msg)
{
    sdk_ret_t ret = SDK_RET_OK;

    sdk::ipc::request(core::PDS_THREAD_ID_API, api::API_MSG_ID_BATCH, api_msg,
                      sizeof(*api_msg), process_sync_result, &ret);
    return ret;
}

static sdk_ret_t
process_deleted_objects (learn_entry_list_t *del_objects)
{
    sdk_ret_t ret;

    for (auto it = del_objects->begin(); it != del_objects->end(); ++it) {
        learn_entry_t del_obj = *it;
        switch (del_obj.obj_type) {
        case LEARN_ENTRY_TYPE_MAC:
            ret = delete_mac_entry(del_obj.mac_entry);
            break;
        case LEARN_ENTRY_TYPE_IP:
            ret = delete_ip_entry(del_obj.ip_entry, del_obj.mac_entry);
            break;
        default:
            SDK_ASSERT(false);
        }
        if (ret != SDK_RET_OK) {
            PDS_TRACE_DEBUG("Failed to process deleted object list of size %u,"
                            " return code %u", del_objects->size(), ret);
            return ret;
        }
    }
    PDS_TRACE_DEBUG("Processed deleted object list of size %u, return code %u",
                    del_objects->size(), ret);
    return SDK_RET_OK;
}

static sdk_ret_t
process_api (api_ctxt_t *api_ctxt, pds_batch_ctxt_t bctxt,
             learn_batch_ctxt_t *lbctxt)
{
    mapping_key_spec_t key_spec;

    // only mapping object is supported
    if (api_ctxt->obj_id != OBJ_ID_MAPPING) {
        return SDK_RET_INVALID_OP;
    }
    switch (api_ctxt->api_op) {
    case API_OP_CREATE:
    case API_OP_UPDATE:
        key_spec.spec = &api_ctxt->api_params->mapping_spec;
        break;
    case API_OP_DELETE:
        key_spec.skey = &api_ctxt->api_params->mapping_skey;
        break;
    default:
        return SDK_RET_INVALID_OP;
    }
    return process_mapping_api(key_spec, api_ctxt->api_op, bctxt, lbctxt);
}

#ifdef BATCH_SUPPORT
sdk_ret_t
process_api_batch (api::api_msg_t *api_msg)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    learn_batch_ctxt_t lbctxt;
    auto apis = &api_msg->batch.apis;
    pds_batch_params_t batch_params {learn_db()->epoch_next(), false, nullptr,
                                     nullptr};

    // if this is not a mapping API batch, relay it directly to API thread
    if (passthrough_api_batch(api_msg)) {
        PDS_TRACE_DEBUG("Rcvd passthrough API batch");
        return process_passthrough_api_batch(api_msg);
    }

    memset(&lbctxt.counters, 0, sizeof(lbctxt.counters));
    // create a new sync batch
    bctxt = pds_batch_start(&batch_params);
    if (unlikely(bctxt == PDS_BATCH_CTXT_INVALID)) {
        return SDK_RET_OOM;
    }

    // process each API in the batch
    for (auto it = apis->begin(); it != apis->end(); ++it) {
        ret = process_api(*it, bctxt, &lbctxt);
        if (ret != SDK_RET_OK) {
            return ret;
        }
    }

    // commit the new batch
    ret = pds_batch_commit(bctxt);
    update_batch_counters(&lbctxt, ret == SDK_RET_OK);
    if (unlikely(ret != SDK_RET_OK)) {
        return ret;
    }

    // batch commit succeeded, send events, clean up state for deleted objects
    broadcast_events(&lbctxt);
    ret = process_deleted_objects(&lbctxt.del_objs);
    return ret;
}
#else
// temporary workround until we resolve batching issues for mapping object
sdk_ret_t
process_api_batch (api::api_msg_t *api_msg)
{
    sdk_ret_t ret;
    learn_batch_ctxt_t lbctxt;
    auto apis = &api_msg->batch.apis;

    // if this is not a mapping API batch, relay it directly to API thread
    if (passthrough_api_batch(api_msg)) {
        PDS_TRACE_DEBUG("Rcvd passthrough API batch");
        return process_passthrough_api_batch(api_msg);
    }

    // process each API in the batch individually
    for (auto it = apis->begin(); it != apis->end(); ++it) {
        lbctxt.reset();
        ret = process_api(*it, PDS_BATCH_CTXT_INVALID, &lbctxt);
        update_batch_counters(&lbctxt, ret == SDK_RET_OK);
        if (ret != SDK_RET_OK) {
            return ret;
        }
        broadcast_events(&lbctxt);
        ret = process_deleted_objects(&lbctxt.del_objs);
        if (unlikely(ret != SDK_RET_OK)) {
            return ret;
        }
    }
    return SDK_RET_OK;
}
#endif

}   // namespace learn
