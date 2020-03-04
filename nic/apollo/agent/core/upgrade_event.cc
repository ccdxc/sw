//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/core/core.hpp"
#include "nic/apollo/upgrade/include/upgrade.hpp"
#include "nic/apollo/api/include/pds_upgrade.hpp"

namespace core {

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

// upgrade event handler
// these events are coming from upgrade manager over sdk IPC
static void
upg_event_handler (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    upg_event_msg_t *event = (upg_event_msg_t *)msg->data();
    upg_event_msg_t resp;
    pds_upg_spec_t  spec;
    sdk_ret_t       ret = SDK_RET_OK;

    PDS_TRACE_DEBUG("Received UPG IPC event stage {}",
                    upg_stage2str(event->stage));

    // TODO fill other infos
    spec.stage = event->stage;
    ret = pds_upgrade(&spec);
    resp.stage = event->stage;
    resp.rsp_status = sdk_ret_to_upg_status(ret);
    strncpy(resp.rsp_thread_name, UPG_PDS_AGENT_NAME, sizeof(resp.rsp_thread_name));
    // invoked by service thread
    resp.rsp_thread_id = PDS_AGENT_THREAD_ID_SVC_SERVER;
    // respond to upgrade manager with unicast message
    sdk::ipc::respond(msg, &resp, sizeof(resp));
}

void
upg_event_subscribe (void)
{
    // subscribe for upgrade events from upgrade manager
    // for both broadcast and unicast messages
    sdk::ipc::subscribe(PDS_IPC_MSG_ID_UPGRADE, upg_event_handler, NULL);
    sdk::ipc::reg_request_handler(PDS_IPC_MSG_ID_UPGRADE, upg_event_handler, NULL);
}

}    // namespace core
