//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/apollo/core/core.hpp"
#include "nic/apollo/core/event.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/internal/upgrade_ev.hpp"
#include "nic/apollo/api/upgrade.hpp"

using api::upg_ev_params_t;

namespace nicmgr {

static sdk_ret_t
nicmgr_send_ipc (api::upg_ev_msg_id_t id)
{
    // TODO
    return SDK_RET_IN_PROGRESS;
}

static sdk_ret_t
upg_ev_compat_check (upg_ev_params_t *params)
{
    return nicmgr_send_ipc(UPG_MSG_ID_COMPAT_CHECK);
}

static sdk_ret_t
upg_ev_ready (upg_ev_params_t *params)
{
    return nicmgr_send_ipc(UPG_MSG_ID_READY);
}

static sdk_ret_t
upg_ev_backup (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_quiesce (upg_ev_params_t *params)
{
    return nicmgr_send_ipc(UPG_MSG_ID_QUIESCE);
}

static sdk_ret_t
upg_ev_switchover (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_repeal (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_exit (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

sdk_ret_t
upg_hitless_init (void)
{
    upg_ev_hitless_t ev_hdlr;

    // fill upgrade events
    memset(&ev_hdlr, 0, sizeof(ev_hdlr));
    strncpy(ev_hdlr.thread_name, "nicmgr", sizeof(ev_hdlr.thread_name));
    ev_hdlr.compat_check_hdlr = upg_ev_compat_check;
    ev_hdlr.ready_hdlr = upg_ev_ready;
    ev_hdlr.backup_hdlr = upg_ev_backup;
    ev_hdlr.repeal_hdlr = upg_ev_repeal;
    ev_hdlr.quiesce_hdlr = upg_ev_quiesce;
    ev_hdlr.switchover_hdlr = upg_ev_switchover;
    ev_hdlr.repeal_hdlr = upg_ev_repeal;
    ev_hdlr.exit_hdlr = upg_ev_exit;

    // register for upgrade events
    api::upg_ev_thread_hdlr_register(ev_hdlr);

    return SDK_RET_OK;
}

}    // namespace nicmgr
