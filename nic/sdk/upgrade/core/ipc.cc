// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// common definitions for core upgrade data structure for external uses
///
//----------------------------------------------------------------------------

#include "include/sdk/base.hpp"
#include "lib/ipc/ipc.hpp"
#include "upgrade/include/ev.hpp"
#include "ipc.hpp"
#include "logger.hpp"

namespace sdk {
namespace upg {

static upg_async_response_cb_t response_cb;

void
upg_send_request (ipc_svc_dom_id_t dom_id, upg_stage_t stage, uint32_t svc_id)
{
    upg_event_msg_t msg;
    upg_ev_id_t ev_id = upg_stage2event(stage);

    msg.stage = stage;

    UPG_TRACE_INFO("Request notification stage %s, event %s, svcid %u\n",
                   upg_stage2str(msg.stage), upg_event2str(ev_id), svc_id);
    sdk::ipc::request(svc_id, ev_id, &msg, sizeof(msg), NULL);
}

void
upg_send_broadcast_request (ipc_svc_dom_id_t dom_id, upg_stage_t stage)
{
    upg_event_msg_t msg;
    upg_ev_id_t ev_id = upg_stage2event(stage);

    msg.stage = stage;

    UPG_TRACE_INFO("Broadcasting notification for stage %s, event %s\n",
                   upg_stage2str(msg.stage), upg_event2str(ev_id));
    sdk::ipc::broadcast(ev_id, &msg, sizeof(msg));
}

static void
upg_ev_handler (sdk::ipc::ipc_msg_ptr msg, const void *req_cookie,
                const void *ctxt)
{
    response_cb(msg);
}

void
upg_ipc_init (upg_async_response_cb_t rsp_cb)
{
    for (uint32_t ev_id = UPG_EV_COMPAT_CHECK; ev_id < UPG_EV_MAX; ev_id++) {
        sdk::ipc::reg_response_handler(ev_id, upg_ev_handler, NULL);
    }
    response_cb = rsp_cb;
}

}    // namespace upg
}    // namespace sdk
