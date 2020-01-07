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

namespace hal {
namespace svc {

extern std::shared_ptr<delphi_client>    g_delphic;

}    // namespace svc
}    // namespace hal

namespace hal {
namespace upgrade {

typedef struct ipc_watcher_s {
    sdk::ipc::handler_cb callback;
    const void *ctx;
} ipc_watcher_t;

static int upg_async_wakeup_fd[2];

static void
upg_process_sync_result (sdk::ipc::ipc_msg_ptr msg, const void *status)
{
    upg_msg_t *dst = (upg_msg_t *)status;
    upg_msg_t *src = (upg_msg_t *)msg->data();

    dst->msg_id = src->msg_id;
    dst->rsp_code = src->rsp_code;
    strncpy(dst->rsp_err_string, src->rsp_err_string, sizeof(dst->rsp_err_string));
    HAL_TRACE_DEBUG("[upgrade] result msg {} rspcode {} err {}",
                    dst->msg_id, dst->rsp_code, dst->rsp_err_string);
}

HdlrResp
upg_event_notify (upg_msg_id_t id, UpgCtx& upgCtx)
{
    upg_msg_t msg;
    upg_msg_t status;
    HdlrResp rsp;

    msg.msg_id = id;
    msg.prev_exec_state = upgCtx.prevExecState;
    status.rsp_code = UPG_RSP_PENDING;

    // makesure nicmgr is ready. otherwise return error
    if (!hal_thread_ready(hal::HAL_THREAD_ID_NICMGR)) {
        HAL_TRACE_ERR("Nicmgr not up, upgrade failed");
        goto end;
    }
    HAL_TRACE_DEBUG("[upgrade] sendin msg {} to nicmgr", id);

    // on delphic thread this is a sync request
    // need to wait for the responses
    sdk::ipc::request(hal::HAL_THREAD_ID_NICMGR, event_id_t::EVENT_ID_UPG, &msg, sizeof(msg),
                      upg_process_sync_result, &status);
    HAL_TRACE_DEBUG("[upgrade] result.. msg {} rspcode {} err {}",
                    status.msg_id, status.rsp_code, status.rsp_err_string);

end:
    switch(status.rsp_code) {
    case UPG_RSP_SUCCESS:
        rsp.resp = ::upgrade::SUCCESS;
        break;
    case UPG_RSP_INPROGRESS:
        rsp.resp = ::upgrade::INPROGRESS;
        break;
    case UPG_RSP_FAIL:
    default:
        rsp.resp = ::upgrade::FAIL;
        break;
    }
    rsp.errStr = std::string(status.rsp_err_string);
    return rsp;
}


HdlrResp
combine_responses (HdlrResp &nicmgr_rsp, HdlrResp &rsp)
{
    HAL_TRACE_DEBUG("[upgrade] combine response nicmgr {} {} hal {} {}",
                     nicmgr_rsp.resp, nicmgr_rsp.errStr,
                     rsp.resp, rsp.errStr);
    // if both are success
    if (nicmgr_rsp.resp == ::upgrade::SUCCESS && rsp.resp == ::upgrade::SUCCESS) {
        return rsp;
    }
    // return failure if either of them failed
    if (nicmgr_rsp.resp == ::upgrade::FAIL) {
        return nicmgr_rsp;
    }
    if (rsp.resp == ::upgrade::FAIL) {
        return rsp;
    }
    // return in progress if either of them in progress
    if (nicmgr_rsp.resp == ::upgrade::INPROGRESS) {
        return nicmgr_rsp;
    }
    if (rsp.resp == ::upgrade::INPROGRESS) {
        return rsp;
    }
    // should never come here
    SDK_ASSERT(0);
}

#if 0
// below code has been taken from sdk/lib/event_thread.cc
static void
ipc_io_callback (struct ev_loop *loop, ev_io *watcher, int revents)
{
    ipc_watcher_t *ipc_watcher = (ipc_watcher_t *)watcher->data;

    ipc_watcher->callback(watcher->fd, ipc_watcher->ctx);
}

static void
create_ipc_watcher (int fd, sdk::ipc::handler_cb cb,
                    const void *ctx,
                    const void *ipc_poll_fd_ctx)
{
    // todo: fix me: we are leaking!
    ipc_watcher_t *ipc_watcher = (ipc_watcher_t *)malloc(sizeof(*ipc_watcher));
    ipc_watcher->callback = cb;
    ipc_watcher->ctx = ctx;

    // todo: fix me: we are leaking!
    ev_io *watcher = (ev_io *)malloc(sizeof(*watcher));
    watcher->data = ipc_watcher;

    ev_io_init(watcher, ipc_io_callback, fd, EV_READ);
    ev_io_start(EV_DEFAULT, watcher);
}

static void
upg_stage_event_handler (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    bool status  = *(bool *)msg->data();

    HAL_TRACE_DEBUG("Received upgrade inprogress status {} from nicmgr", status);
    if (!hal::svc::g_delphic) {
        return;
    }
    hal::svc::g_delphic->send_upg_stage_status(status);
}
#endif

static void
upg_async_cb (struct ev_loop *loop, ev_io *watcher, int revents)
{
    char buf;
    bool status;

    HAL_TRACE_DEBUG("Upgrade async callback");
    read(upg_async_wakeup_fd[0], &buf, sizeof(buf));
    status = buf == '1' ? true : false;

    HAL_TRACE_DEBUG("Sending upg stage status {}", status);
    if (hal::svc::g_delphic) {
        hal::svc::g_delphic->send_upg_stage_status(status);
    }
}

void
upg_async_status_update (bool status)
{
    char buf = status ? '1' : '2';

    HAL_TRACE_DEBUG("Received upgrade inprogress status {} from nicmgr", status);
    write(upg_async_wakeup_fd[1], &buf, 1);
}


// called by delphi thread
// uses default event loop for ipc
void
upg_event_init (void)
{
    // not possible to do async as hal to nicmgr should be sync
    // sdk::ipc::ipc_init_async(hal::HAL_THREAD_ID_DELPHI_CLIENT, create_ipc_watcher, NULL);
    // nicmgr sends async events for INPROGRESS events
    // sdk::ipc::subscribe(event_id_t::EVENT_ID_UPG_STAGE_STATUS, upg_stage_event_handler, NULL);
    SDK_ASSERT(pipe(upg_async_wakeup_fd) == 0);

    ev_io *watcher = (ev_io *)malloc(sizeof(*watcher));
    watcher->data = NULL;

    ev_io_init(watcher, upg_async_cb, upg_async_wakeup_fd[0], EV_READ);
    ev_io_start(EV_DEFAULT, watcher);
}

}    // namespace upgrade
}    // namespace hal
