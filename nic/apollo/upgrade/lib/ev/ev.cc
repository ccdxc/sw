//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <string.h>
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/apollo/upgrade/include/ev.hpp"

namespace upg {

static upg_ev_t upg_ev;

static inline upg_status_t
sdk_ret_to_upg_status (sdk_ret_t sdk_ret)
{
    switch (sdk_ret) {
    case SDK_RET_OK:
        return UPG_STATUS_OK;
    case sdk_ret_t::SDK_RET_UPG_CRITICAL:
        return UPG_STATUS_CRITICAL;
    default:
        return UPG_STATUS_FAIL;
    }
}

// ipc incoming msg pointer which should be saved and later
// used to respond to upgmgr
typedef struct upg_ev_cookie_s {
    sdk::ipc::ipc_msg_ptr msg;
} upg_ev_cookie_t;


static upg_ev_id_t
spec2evid (upg_stage_t stage, upg_ev_hdlr_t *hdlr)
{
    switch(stage) {
    case UPG_STAGE_START: {
        *hdlr = upg_ev.start;
        return UPG_EV_START;
    }
    // TODO fill remaining
    default:
        SDK_TRACE_DEBUG("Upgrade unknown stageid %u", stage);
        return UPG_EV_NONE;
    }
}

// ipc async response callbacks
static void
upg_ev_response_cb (sdk_ret_t status, const void *cookie)
{
    upg_event_msg_t resp;
    upg_ev_cookie_t *ck = (upg_ev_cookie_t *)cookie;
    upg_event_msg_t *event = (upg_event_msg_t *)ck->msg->data();

    resp.stage = event->stage;
    resp.rsp_status = sdk_ret_to_upg_status(status);
    strncpy(resp.rsp_thread_name, upg_ev.svc_name, sizeof(resp.rsp_thread_name));
    // invoked by service thread
    resp.rsp_thread_id = upg_ev.svc_id;
    // respond to upgrade manager with unicast message
    sdk::ipc::respond(ck->msg, &resp, sizeof(resp));
    SDK_TRACE_DEBUG("Upgrade IPC response stage {} completed status {}",
                    upg_stage2str(event->stage), resp.rsp_status);
    delete ck;
}

// upgrade event handler
// these events are coming from upgrade manager over sdk IPC
static void
upg_ev_handler (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    upg_event_msg_t *event = (upg_event_msg_t *)msg->data();
    upg_ev_params_t params;
    sdk_ret_t ret;
    upg_ev_cookie_t *cookie = new upg_ev_cookie_t();
    upg_ev_hdlr_t ev_func;
    upg_ev_id_t ev_id;

    if (event->stage >= UPG_STAGE_MAX) {
        SDK_TRACE_ERR("Upgrade Invalid stage id %u", event->stage);
        // returning OK as latest upgmgr and prev services should work
        return upg_ev_response_cb(SDK_RET_OK, cookie);
    }

    ev_id = spec2evid(event->stage, &ev_func);
    if (ev_id == UPG_EV_NONE) {
        // returning OK as latest upgmgr and prev services should work
        return upg_ev_response_cb(SDK_RET_OK, cookie);
    }

    // validate event-id vs stage
    SDK_TRACE_DEBUG("Upgrade IPC event stage {} starting..",
                    upg_stage2str(event->stage));

    // TODO fill other infos
    cookie->msg = msg;
    params.id = ev_id;
    params.mode = event->mode;
    params.response_cb = upg_ev_response_cb;
    params.response_cookie = cookie;
    ret = ev_func(&params);
    SDK_TRACE_DEBUG("Upgrade IPC event stage {} ret {}",
                    upg_stage2str(event->stage), ret);
    if (ret != SDK_RET_IN_PROGRESS) {
        upg_ev_response_cb(ret, cookie);
    }
    return;
}

void
upg_ev_hdlr_register(upg_ev_t &ev)
{
    memcpy(&upg_ev, &ev, sizeof(upg_ev_t));

    // subscribe for upgrade events from upgrade manager
    // below 2 are broadcast events and it is mandatory to be provided
    SDK_ASSERT(ev.start);
    SDK_ASSERT(ev.ready);
    sdk::ipc::subscribe(PDS_IPC_MSG_ID_UPG_COMPAT_CHECK, upg_ev_handler, NULL);
    sdk::ipc::subscribe(PDS_IPC_MSG_ID_UPG_READY, upg_ev_handler, NULL);

    // below are unicast events
    sdk::ipc::reg_request_handler(PDS_IPC_MSG_ID_UPG_COMPAT_CHECK,
                                  upg_ev_handler, NULL);
    sdk::ipc::reg_request_handler(PDS_IPC_MSG_ID_UPG_START,
                                  upg_ev_handler, NULL);
    sdk::ipc::reg_request_handler(PDS_IPC_MSG_ID_UPG_BACKUP,
                                  upg_ev_handler, NULL);
    sdk::ipc::reg_request_handler(PDS_IPC_MSG_ID_UPG_PREPARE,
                                  upg_ev_handler, NULL);
    sdk::ipc::reg_request_handler(PDS_IPC_MSG_ID_UPG_READY,
                                  upg_ev_handler, NULL);
    sdk::ipc::reg_request_handler(PDS_IPC_MSG_ID_UPG_SYNC,
                                  upg_ev_handler, NULL);
    sdk::ipc::reg_request_handler(PDS_IPC_MSG_ID_UPG_PREPARE_SWITCHOVER,
                                  upg_ev_handler, NULL);
    sdk::ipc::reg_request_handler(PDS_IPC_MSG_ID_UPG_SWITCHOVER,
                                  upg_ev_handler, NULL);
    sdk::ipc::reg_request_handler(PDS_IPC_MSG_ID_UPG_ROLLBACK,
                                  upg_ev_handler, NULL);
    sdk::ipc::reg_request_handler(PDS_IPC_MSG_ID_UPG_REPEAL,
                                  upg_ev_handler, NULL);
    sdk::ipc::reg_request_handler(PDS_IPC_MSG_ID_UPG_FINISH,
                                  upg_ev_handler, NULL);
    sdk::ipc::reg_request_handler(PDS_IPC_MSG_ID_UPG_EXIT,
                                  upg_ev_handler, NULL);
}

}   // namespace upg
