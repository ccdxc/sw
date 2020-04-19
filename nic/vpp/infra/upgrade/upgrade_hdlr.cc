//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include "nic/apollo/include/globals.hpp"
#include "infra/api/intf.h"
#include "log.hpp"
#include "upgrade_hdlr.hpp"

static const char *grupg_str = "Graceful Upgrade";

static void
vpp_upg_ev_fill (sdk::upg::upg_ev_t &ev)
{
    ev.svc_ipc_id = PDS_IPC_ID_VPP;
    strncpy(ev.svc_name, "vpp", sizeof(ev.svc_name));
    ev.compat_check_hdlr = vpp_upg_ev_hdlr;
    ev.start_hdlr = vpp_upg_ev_hdlr;
    ev.backup_hdlr = vpp_upg_ev_hdlr;
    ev.prepare_hdlr = vpp_upg_ev_hdlr;
    ev.prepare_switchover_hdlr = vpp_upg_ev_hdlr;
    ev.switchover_hdlr = vpp_upg_ev_hdlr;
    ev.rollback_hdlr = vpp_upg_ev_hdlr;
    ev.sync_hdlr = vpp_upg_ev_hdlr;
    ev.respawn_hdlr = vpp_upg_ev_hdlr;
    ev.ready_hdlr = vpp_upg_ev_hdlr;
    ev.repeal_hdlr = vpp_upg_ev_hdlr;
    ev.finish_hdlr = vpp_upg_ev_hdlr;
}

// Hitless restart logic in VPP is quite simple.
//   In prepare stage we shut down cpu_mnic0
static sdk_ret_t
vpp_graceful_upg_ev_hdlr (sdk::upg::upg_ev_params_t *params)
{
    if (params->id == sdk::upg::UPG_EV_PREPARE) {
        // Bring down all interfaces
        upg_log_notice("%s: %s: Bringing down all interfaces\n", __FUNCTION__,
                        grupg_str);
        pds_infra_set_all_intfs_status(false);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpp_upg_ev_hdlr (sdk::upg::upg_ev_params_t *params)
{
    sdk_ret_t ret;

    if (!params) {
        upg_log_error("%s: %s: Invalid Upgrade notification\n", __FUNCTION__,
                      grupg_str);
        return SDK_RET_INVALID_ARG;
    }
    ret = SDK_RET_OK;
    upg_log_notice("%s: %s: Handling Upgrade stage %u mode %u\n", __FUNCTION__,
                   grupg_str, params->id, params->mode);
    if (params->mode == upg_mode_t::UPGRADE_MODE_GRACEFUL) {
        ret = vpp_graceful_upg_ev_hdlr(params);
    } else if (params->mode == upg_mode_t::UPGRADE_MODE_HITLESS) {
    }
    return ret;
}

void
vpp_upg_init (void)
{
    sdk::upg::upg_ev_t ev;

    memset(&ev, 0, sizeof(ev));
    upg_log_facility_init();
    upg_log_notice("%s\n", __FUNCTION__);
    vpp_upg_ev_fill(ev);
    sdk::upg::upg_ev_hdlr_register(ev);
}

