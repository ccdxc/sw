//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// API thread specific helper functions
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/core.hpp"
#include "nic/apollo/framework/api_thread.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"

namespace api {

void
api_thread_init_fn (void *ctxt)
{
    api_engine_init();
    sdk::ipc::reg_request_handler(API_MSG_ID_BATCH, api_thread_ipc_batch_cb,
                                  NULL, true);
}

void
api_thread_exit_fn (void *ctxt)
{
}

void
api_thread_event_cb (void *msg, void *ctxt)
{
}

void
api_thread_ipc_batch_cb (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    sdk_ret_t ret;
    api_msg_t *api_msg = (api_msg_t *)msg->data();

    PDS_TRACE_DEBUG("Rcvd API batch msg");
    // basic validation
    assert(likely(api_msg != NULL));
    assert(likely(api_msg->msg_id == API_MSG_ID_BATCH));
    ret = api_engine_get()->batch_commit(api_msg, msg);
    // if batch is still in progress, don't send response back yet
    if (ret != sdk::SDK_RET_IN_PROGRESS) {
        sdk::ipc::respond(msg, &ret, sizeof(ret));
    }
}

bool
is_api_thread_ready (void) {
    return core::thread_get(core::PDS_THREAD_ID_API)->ready();
}

}    // namespace api
