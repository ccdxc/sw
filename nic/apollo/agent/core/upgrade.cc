//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/upgrade/upgrade.hpp"
#include "nic/apollo/api/include/pds_upgrade.hpp"

namespace core {

// don't change this as this is upgrade dependent
#define UPG_PDS_AGENT_NAME "pdsagent"

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

//------------------------------------------------------------------------------
// register for upgrade specific events
//------------------------------------------------------------------------------
static void
upg_event_handler (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    upg_event_msg_t *event = (upg_event_msg_t *)msg->data();
    upg_event_msg_t rspev;
    pds_upg_spec_t  spec;
    sdk_ret_t       ret = SDK_RET_OK;

    PDS_TRACE_DEBUG("Received UPG IPC event stage %s",
                    upg_stage2str(event->stage));

    // TODO fill other infos
    spec.stage = event->stage;
    ret = pds_upgrade(&spec);
    rspev.rsp_status = sdk_ret_to_upg_status(ret);
    rspev.stage = event->stage;
    strncpy(rspev.rsp_thread_name, UPG_PDS_AGENT_NAME, sizeof(rspev.rsp_thread_name));
    sdk::ipc::broadcast(UPG_EVENT_ID_RSP, &rspev, sizeof(rspev));
}

void
upg_event_subscribe (void)
{
    // subscribe for upgrade client poll.
    sdk::ipc::subscribe(UPG_EVENT_ID_REQ, upg_event_handler, NULL);
}

}    // namespace core
