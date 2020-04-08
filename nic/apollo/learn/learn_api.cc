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

sdk_ret_t
api_batch_commit (pds_batch_ctxt_t bctxt)
{
    api::api_msg_t *api_msg = (api::api_msg_t *)bctxt;

    // if learning is disabled, directly commit to API thread
    if (!learning_enabled()) {
        PDS_TRACE_DEBUG("Learning is disabled, committing API batch directly");
        return pds_batch_commit(bctxt);
    }

    if (likely(api_msg->batch.apis.size() > 0)) {
        //TODO: sync batch support
        if (!api_msg->batch.async) {
            return SDK_RET_INVALID_OP;
        }
        sdk::ipc::request(core::PDS_THREAD_ID_LEARN, LEARN_MSG_ID_API,
                          api_msg, sizeof(*api_msg), process_async_result,
                          api_msg);
    } else {
        pds_batch_destroy(bctxt);
    }
    return SDK_RET_OK;
}

static sdk_ret_t
process_deleted_objects (learn_entry_list_t *del_objects)
{
    sdk_ret_t ret = SDK_RET_OK;

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
            return SDK_RET_ERR;
        }
        if (ret != SDK_RET_OK) {
            return ret;
        }
    }
    return ret;
}

static sdk_ret_t
process_api (api_ctxt_t *api_ctxt, pds_batch_ctxt_t bctxt,
             learn_entry_list_t *del_obj_list)
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
    return process_mapping_api(key_spec, api_ctxt->api_op, bctxt, del_obj_list);
}

#ifdef BATCH_SUPPORT
sdk_ret_t
process_api_batch (api::api_msg_t *api_msg)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    learn_entry_list_t del_obj_list;
    auto apis = &api_msg->batch.apis;
    pds_batch_params_t batch_params {learn_db()->epoch_next(), false, nullptr,
                                     nullptr};

    // create a new sync batch
    bctxt = pds_batch_start(&batch_params);
    if (unlikely(bctxt == PDS_BATCH_CTXT_INVALID)) {
        return SDK_RET_OOM;
    }

    // process each API in the batch
    for (auto it = apis->begin(); it != apis->end(); ++it) {
        ret = process_api(*it, bctxt, &del_obj_list);
        if (ret != SDK_RET_OK) {
            return ret;
        }
    }

    // commit the new batch
    ret = pds_batch_commit(bctxt);
    if (unlikely(ret != SDK_RET_OK)) {
        return ret;
    }

    // batch commit succeeded, clean up state for deleted objects
    ret = process_deleted_objects(&del_obj_list);
    return ret;
}
#else
// temporary workround until we resolve batching issues for mapping object
sdk_ret_t
process_api_batch (api::api_msg_t *api_msg)
{
    sdk_ret_t ret;
    auto apis = &api_msg->batch.apis;
    learn_entry_list_t del_obj_list;

    // process each API in the batch individually
    for (auto it = apis->begin(); it != apis->end(); ++it) {
        del_obj_list.clear();
        ret = process_api(*it, PDS_BATCH_CTXT_INVALID, &del_obj_list);
        if (ret != SDK_RET_OK) {
            return ret;
        }
        ret = process_deleted_objects(&del_obj_list);
        if (unlikely(ret != SDK_RET_OK)) {
            return ret;
        }
    }
    return SDK_RET_OK;
}
#endif

}   // namespace learn
