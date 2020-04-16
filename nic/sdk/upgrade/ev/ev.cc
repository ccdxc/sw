//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <string.h>
#include "lib/ipc/ipc.hpp"
#include "upgrade/include/ev.hpp"

namespace sdk {
namespace upg {

static upg_ev_t upg_ev;
static uint64_t upg_ev_bitmap;

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
typedef struct upg_ev_info_s {
    sdk::ipc::ipc_msg_ptr msg_in;
} upg_ev_info_t;

static inline upg_ev_hdlr_t
upg_event2hdlr (upg_ev_id_t ev_id)
{
    switch(ev_id) {
    case UPG_EV_COMPAT_CHECK    : return upg_ev.compat_check_hdlr;
    case UPG_EV_START           : return upg_ev.start_hdlr;
    case UPG_EV_BACKUP          : return upg_ev.backup_hdlr;
    case UPG_EV_PREPARE         : return upg_ev.prepare_hdlr;
    case UPG_EV_SYNC            : return upg_ev.sync_hdlr;
    case UPG_EV_PREP_SWITCHOVER : return upg_ev.prepare_switchover_hdlr;
    case UPG_EV_SWITCHOVER      : return upg_ev.switchover_hdlr;
    case UPG_EV_READY           : return upg_ev.ready_hdlr;
    case UPG_EV_RESPAWN         : return upg_ev.respawn_hdlr;
    case UPG_EV_ROLLBACK        : return upg_ev.rollback_hdlr;
    case UPG_EV_REPEAL          : return upg_ev.repeal_hdlr;
    case UPG_EV_FINISH          : return upg_ev.finish_hdlr;
    default:
        SDK_TRACE_DEBUG("Upgrade unknown event %s", upg_event2str(ev_id));
        SDK_ASSERT(0);
    }
}

// ipc async response processing function
static void
upg_ev_process_response (sdk_ret_t status, const void *cookie)
{
    upg_event_msg_t resp;
    upg_ev_info_t *info = (upg_ev_info_t *)cookie;
    upg_event_msg_t *event = (upg_event_msg_t *)info->msg_in->data();

    // upgrademgr not expecting in-progress status
    if (status == SDK_RET_IN_PROGRESS) {
        SDK_TRACE_INFO("Upgrade ignoring inprogress result");
        return;
    }

    resp.stage = event->stage;
    resp.rsp_status = sdk_ret_to_upg_status(status);
    strncpy(resp.rsp_svc_name, upg_ev.svc_name, sizeof(resp.rsp_svc_name));
    resp.rsp_ev_bitmap = upg_ev_bitmap;
    // invoked by service thread
    resp.rsp_svc_ipc_id = upg_ev.svc_ipc_id;
    // respond to upgrade manager with unicast message
    sdk::ipc::respond(info->msg_in, &resp, sizeof(resp));
    SDK_TRACE_DEBUG("Upgrade IPC response stage %s completed status %u",
                    upg_stage2str(event->stage), resp.rsp_status);
    delete info;
}

// upgrade event handler
// these events are coming from upgrade manager over sdk IPC
static void
upg_ev_handler (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    upg_event_msg_t *event = (upg_event_msg_t *)msg->data();
    upg_ev_params_t params;
    sdk_ret_t ret;
    upg_ev_info_t *info = new upg_ev_info_t();
    upg_ev_hdlr_t ev_func;
    upg_ev_id_t ev_id;

    info->msg_in = msg;

    if (event->stage >= UPG_STAGE_MAX) {
        SDK_TRACE_INFO("Upgrade Invalid stage %s, ignoring",
                       upg_stage2str(event->stage));
        // returning OK as latest upgmgr and prev services should work
        return upg_ev_process_response(SDK_RET_OK, info);
    }

    ev_id = upg_stage2event(event->stage);
    ev_func = upg_event2hdlr(ev_id);

    // if it not implemented, just return OK
    if (!ev_func) {
        SDK_TRACE_INFO("Upgrade stage %s not implemented %s",
                       upg_stage2str(event->stage));
        return upg_ev_process_response(SDK_RET_OK, info);
    }

    // validate event-id vs stage
    SDK_TRACE_DEBUG("Upgrade IPC event stage %s starting..",
                    upg_stage2str(event->stage));

    // TODO fill other infos
    params.id = ev_id;
    params.mode = event->mode;
    params.response_cb = upg_ev_process_response;
    params.response_cookie = info;
    ret = ev_func(&params);
    SDK_TRACE_DEBUG("Upgrade IPC event stage %s ret %u",
                    upg_stage2str(event->stage), ret);
    if (ret != SDK_RET_IN_PROGRESS) {
        upg_ev_process_response(ret, info);
    }
    return;
}

void
upg_ev_hdlr_register(upg_ev_t &ev)
{
    memcpy(&upg_ev, &ev, sizeof(upg_ev_t));

    // subscribe for upgrade events from upgrade manager
    // below 2 are broadcast events and it is mandatory to be provided
    SDK_ASSERT(ev.compat_check_hdlr);
    SDK_ASSERT(ev.ready_hdlr);
    sdk::ipc::subscribe(UPG_EV_COMPAT_CHECK, upg_ev_handler, NULL);
    sdk::ipc::subscribe(UPG_EV_READY, upg_ev_handler, NULL);

    // below are unicast events
    // registering handlers for even not used ones to work with latest upgmr
    // and prev version of services
    for(uint32_t ev_id = UPG_EV_COMPAT_CHECK; ev_id <= UPG_EV_MAX; ev_id++) {
        sdk::ipc::reg_request_handler(ev_id, upg_ev_handler, NULL);
        upg_ev_bitmap |= (1 << ev_id);
    }
}

}   // namespace upg
}   // namespace sdk
