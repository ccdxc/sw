//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/apollo/core/core.hpp"
#include "nic/apollo/core/event.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/internal/upgrade_ev.hpp"
#include "nic/apollo/api/upgrade.hpp"

namespace nicmgr {

static sdk_ret_t
nicmgr_send_ipc (event_id_t id)
{
    // TODO
    return SDK_RET_IN_PROGRESS;
}

static sdk_ret_t
upg_ev_compat_check (upg_ev_params_t *params)
{
    return nicmgr_send_ipc(EVENT_ID_UPG_COMPAT_CHECK);
}

static sdk_ret_t
upg_ev_start (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_backup (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_repeal (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_quiesce (upg_ev_params_t *params)
{
    return nicmgr_send_ipc(EVENT_ID_UPG_QUIESCE);
}

static sdk_ret_t
upg_ev_ready (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_switchover (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_link_down (upg_ev_params_t *params)
{
    return nicmgr_send_ipc(EVENT_ID_UPG_LINK_DOWN);
}

static sdk_ret_t
upg_ev_hostdev_reset (upg_ev_params_t *params)
{
    return nicmgr_send_ipc(EVENT_ID_UPG_HOSTDEV_RESET);
}

static sdk_ret_t
upg_ev_exit (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

sdk_ret_t
upg_graceful_init (void)
{
    upg_ev_graceful_t ev_hdlr;

    // fill upgrade events
    memset(&ev_hdlr, 0, sizeof(ev_hdlr));
    ev_hdlr.thread_id = core::PDS_THREAD_ID_NICMGR;
    ev_hdlr.compat_check = upg_ev_compat_check;
    ev_hdlr.start = upg_ev_start;
    ev_hdlr.backup = upg_ev_backup;
    ev_hdlr.linkdown = upg_ev_link_down;
    ev_hdlr.hostdev_reset = upg_ev_hostdev_reset;
    ev_hdlr.ready  = upg_ev_ready;
    ev_hdlr.quiesce = upg_ev_quiesce;
    ev_hdlr.switchover = upg_ev_switchover;
    ev_hdlr.repeal = upg_ev_repeal;
    ev_hdlr.exit  = upg_ev_exit;

    // register for upgrade events
    api::upg_ev_thread_hdlr_register(ev_hdlr);

    return SDK_RET_OK;
}

}    // namespace nicmgr
