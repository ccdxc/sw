//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines graceful/hitless upgrade implementation for cloud pipeline
///
//----------------------------------------------------------------------------

#include "lib/ipc/ipc_ev.hpp"
#include "upgrade/include/ev.hpp"
#include "platform/pciemgrutils/include/pciesys.h"
#include "platform/pciemgr/include/pciemgr.h"
#include "platform/pciemgrd/pciemgrd_impl.hpp"

static sdk_ret_t
upg_ev_ready_hdlr (sdk::upg::upg_ev_params_t *params)
{
    pciesys_loginfo("Upgrade handler, event %s mode %u\n",
                    sdk::upg::upg_event2str(params->id), params->mode);
    return SDK_RET_OK;
}


static sdk_ret_t
upg_ev_finish_hdlr (sdk::upg::upg_ev_params_t *params)
{
    pciesys_loginfo("Upgrade handler, event %s mode %u\n",
                    sdk::upg::upg_event2str(params->id), params->mode);
    if (params->mode == upg_mode_t::UPGRADE_MODE_GRACEFUL) {
         upgrade_complete();
    }
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_repeal_hdlr (sdk::upg::upg_ev_params_t *params)
{
    pciesys_loginfo("Upgrade handler, event %s mode %u\n",
                    sdk::upg::upg_event2str(params->id), params->mode);
    if (params->mode == upg_mode_t::UPGRADE_MODE_GRACEFUL) {
        upgrade_failed();
    }
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_respawn_hdlr (sdk::upg::upg_ev_params_t *params)
{
    pciesys_loginfo("Upgrade handler, event %s mode %u\n",
                    sdk::upg::upg_event2str(params->id), params->mode);
    if (params->mode == upg_mode_t::UPGRADE_MODE_GRACEFUL) {
        if (upgrade_state_save() < 0) {
            pciesys_logerror("Upgrade handler, state save failed\n");
            return SDK_RET_ERR;
        }
        upgrade_rollback_begin();
    }
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_backup_hdlr (sdk::upg::upg_ev_params_t *params)
{
    pciesys_loginfo("Upgrade handler, event %s mode %u\n",
                    sdk::upg::upg_event2str(params->id), params->mode);
    if (params->mode == upg_mode_t::UPGRADE_MODE_GRACEFUL) {
        if (upgrade_state_save() < 0) {
            pciesys_logerror("Upgrade handler, state save failed\n");
            return SDK_RET_ERR;
        }
    }
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_compat_check_hdlr (sdk::upg::upg_ev_params_t *params)
{
    pciesys_loginfo("Upgrade handler, event %s mode %u\n",
                    sdk::upg::upg_event2str(params->id), params->mode);
    return SDK_RET_OK;
}

static void
upg_ev_fill (sdk::upg::upg_ev_t &ev)
{
    ev.svc_ipc_id = sdk_ipc_id_t::SDK_IPC_ID_PCIEMGR;
    strncpy(ev.svc_name, "pciemgr", sizeof(ev.svc_name));
    ev.compat_check_hdlr = upg_ev_compat_check_hdlr;
    ev.ready_hdlr = upg_ev_ready_hdlr;
    ev.backup_hdlr = upg_ev_backup_hdlr;
    ev.repeal_hdlr = upg_ev_repeal_hdlr;
    ev.respawn_hdlr = upg_ev_respawn_hdlr;
    ev.finish_hdlr = upg_ev_finish_hdlr;
}

void
upg_ev_init (void)
{
    sdk::upg::upg_ev_t ev;

    pciesys_loginfo("Upgrade handler, registering for ipc events\n");
    memset(&ev, 0, sizeof(ev));
    sdk::ipc::ipc_init_ev_default(sdk_ipc_id_t::SDK_IPC_ID_PCIEMGR);
    upg_ev_fill(ev);
    sdk::upg::upg_ev_hdlr_register(ev);
}
