// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// common definitions for core upgrade data structure for external uses
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "lib/operd/operd.hpp"
#include "lib/operd/logger.hpp"
#include "lib/operd/decoder.h"
#include "nic/apollo/upgrade/core/logger.hpp"
#include "nic/apollo/upgrade/include/upgrade.hpp"
#include "nic/apollo/upgrade/core/ipc/ipc.hpp"

namespace upg {
namespace ipc {

static upg_async_response_cb_t response_cb;

void
request (ipc_svc_dom_id_t dom_id, upg_stage_t stage,
        uint32_t svc_id, void *cookie)
{
    upg_event_msg_t msg;

    msg.stage      = stage;
    msg.rsp_cookie = cookie;

    UPG_TRACE_INFO("Request notification stageid %s, svc id %u\n",
                   upg_stage2str(msg.stage), svc_id);

    sdk::ipc::request(svc_id, PDS_IPC_MSG_ID_UPGRADE,
                      &msg, sizeof(msg), NULL);
}

void
broadcast (ipc_svc_dom_id_t dom_id, upg_stage_t stage, void *cookie)
{
    upg_event_msg_t msg;

    msg.stage      = stage;
    msg.rsp_cookie = cookie;

    UPG_TRACE_INFO("Broadcasting notification for stage %s..\n",
                   upg_stage2str(msg.stage));

    sdk::ipc::broadcast(PDS_IPC_MSG_ID_UPGRADE, &msg, sizeof(msg));
}

static void
upg_event_handler (sdk::ipc::ipc_msg_ptr msg, const void *req_cookie,
                   const void *ctxt)
{
    response_cb(msg, req_cookie, ctxt);
}

void
init (upg_async_response_cb_t rsp_cb)
{
    sdk::ipc::reg_response_handler(PDS_IPC_MSG_ID_UPGRADE,
                                   upg_event_handler, NULL);
    response_cb = rsp_cb;
}

}   // namespace ipc
}   // namespace upg

