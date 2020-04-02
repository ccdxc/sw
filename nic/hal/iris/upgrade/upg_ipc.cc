//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <unistd.h>
#include "nic/hal/iris/upgrade/upgrade.hpp"
#include "nic/include/base.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/hal/hal_trace.hpp"
#include "nic/hal/core/core.hpp"
#include "nic/hal/iris/upgrade/upg_ipc.hpp"
#include "nic/hal/iris/delphi/delphic.hpp"
#include "nic/sdk/lib/ipc/ipc_ev.hpp"

namespace hal {
namespace svc {

extern std::shared_ptr<delphi_client>    g_delphic;

}    // namespace svc
}    // namespace hal

namespace hal {
namespace upgrade {

static void
upg_process_async_result (sdk::ipc::ipc_msg_ptr msg, const void *status)
{
    upg_msg_t *src = (upg_msg_t *)msg->data();

    HAL_TRACE_DEBUG("[upgrade] result msg {} rspcode {} err {}",
                    src->msg_id, src->rsp_code, src->rsp_err_string);
    HAL_TRACE_DEBUG("Upgrade async Result");

    if (!hal::svc::g_delphic) {
        return;
    }

    if (src->rsp_code == UPG_RSP_SUCCESS) {
        HAL_TRACE_DEBUG("Received upgrade status {} from nicmgr", true);
        hal::svc::g_delphic->send_upg_stage_status(DELPHIC_UPG_ID_NICMGR, true);
    } else if (src->rsp_code == UPG_RSP_FAIL) {
        HAL_TRACE_DEBUG("Received upgrade status {} from nicmgr", false);
        hal::svc::g_delphic->send_upg_stage_status(DELPHIC_UPG_ID_NICMGR, false);
    }
}

HdlrResp
upg_event_notify (upg_msg_t *msg)
{
    HdlrResp rsp;

    // makesure nicmgr is ready. otherwise return error
    if (!hal_thread_ready(hal::HAL_THREAD_ID_NICMGR)) {
        HAL_TRACE_ERR("Nicmgr not up, upgrade failed");
        rsp.resp = ::upgrade::FAIL;
        goto end;
    }
    HAL_TRACE_DEBUG("[upgrade] sendin msg {} to nicmgr, prevstate {},"
                    "Save State Delphic {}", msg->msg_id, msg->prev_exec_state,
                    msg->save_state_delphi);

    // async request to nicmgr for upgrade
    sdk::ipc::request(hal::HAL_THREAD_ID_NICMGR, event_id_t::EVENT_ID_UPG, msg,
                      sizeof(upg_msg_t), upg_process_async_result, NULL);
    rsp.resp = ::upgrade::INPROGRESS;

end:
    return rsp;
}

void
upg_stage_status_handler (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    bool status  = *(bool *)msg->data();

    HAL_TRACE_DEBUG("Upgrade status async callback");
    HAL_TRACE_DEBUG("Received upgrade status {} from nicmgr", status);
    if (!hal::svc::g_delphic) {
        return;
    }
    hal::svc::g_delphic->send_upg_stage_status(DELPHIC_UPG_ID_NICMGR, status);
}

// called by delphi thread
// uses default event loop for ipc
void
upg_event_init (void)
{
    sdk::ipc::ipc_init_ev_default(hal::HAL_THREAD_ID_DELPHI_CLIENT);
    sdk::ipc::reg_request_handler(event_id_t::EVENT_ID_UPG_STAGE_STATUS,
                                  upg_stage_status_handler, NULL);
}

}    // namespace upgrade
}    // namespace hal
