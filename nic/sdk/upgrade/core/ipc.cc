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
#include "lib/ipc/subscribers.hpp"
#include "upgrade/include/ev.hpp"
#include "ipc.hpp"
#include "logger.hpp"

namespace sdk {
namespace upg {

static upg_async_response_cb_t response_cb;

void
upg_send_request (ipc_svc_dom_id_t dom_id, upg_stage_t stage,
                  uint32_t svc_id, upg_mode_t mode)
{
    upg_event_msg_t msg;
    upg_ev_id_t ev_id = upg_stage2event(stage);

    memset(&msg, 0, sizeof(msg));
    msg.stage = stage;
    msg.mode  = mode;

    UPG_TRACE_INFO("Request notification stage %s, event %s, svcid %u",
                   upg_stage2str(msg.stage), upg_event2str(ev_id), svc_id);
    sdk::ipc::request(svc_id, ev_id, &msg, sizeof(msg), NULL);
}


static sdk_ret_t
wait_for_svc_ready (ipc_svc_dom_id_t dom_id, uint32_t ev_id,
                    uint32_t num_svcs, ev_tstamp timeout)
{
    std::vector<uint32_t> recipients;
    ev_tstamp wait = 0.0;
    uint32_t count = 0;

    while (timeout > wait) {
        // TODO : it is better if IPC layer provides a function for it
        // or brodcast can return number of recipients
        recipients = sdk::ipc::subscribers::instance()->get(ev_id);
        if (recipients.size() < num_svcs) {
            ev_sleep(0.001); // 1 millisecond
            wait += 0.001;
            if (count % 100 == 0) {
                UPG_TRACE_INFO("Waiting for services to be up.., expected %u now %u",
                               num_svcs, (uint32_t)recipients.size());
                count++;
            }
        } else {
            return SDK_RET_OK;
        }
    }
    return SDK_RET_ERR;
}

void
upg_send_broadcast_request (ipc_svc_dom_id_t dom_id, upg_stage_t stage,
                            upg_mode_t mode, uint32_t num_svcs,
                            ev_tstamp timeout)
{
    upg_event_msg_t msg;
    upg_ev_id_t ev_id = upg_stage2event(stage);

    wait_for_svc_ready(dom_id, ev_id, num_svcs, timeout);
    // send the event even if the wait has been failed. fsm timeout will handle.

    memset(&msg, 0, sizeof(msg));
    msg.stage = stage;
    msg.mode  = mode;

    UPG_TRACE_INFO("Broadcasting notification for stage %s, event %s",
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
