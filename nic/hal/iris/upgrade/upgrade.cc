//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/iris/upgrade/upgrade.hpp"
#include "nic/include/base.hpp"
#include "nic/hal/hal_trace.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/linkmgr/linkmgr.hpp"
#include "nic/hal/plugins/cfg/lif/lif.hpp"
#include "nic/hal/src/internal/system.hpp"
#include "nic/hal/iris/upgrade/upg_ipc.hpp"

namespace hal {
namespace upgrade {

std::string empty_str("");

//------------------------------------------------------------------------------
// perform compat check
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::CompatCheckHandler(UpgCtx& upgCtx)
{
    std::string preVer, postVer;
    HdlrResp    nicmgr_rsp, rsp;

    ::upgrade::UpgCtxApi::UpgCtxGetPreUpgTableVersion(upgCtx, ::upgrade::DEVCONFVER, preVer);
    ::upgrade::UpgCtxApi::UpgCtxGetPostUpgTableVersion(upgCtx, ::upgrade::DEVCONFVER, postVer);
    HAL_TRACE_DEBUG("[upgrade] Handling compat checks msg ... preVer {} postVer {}", preVer, postVer);

    nicmgr_rsp = upg_event_notify(MSG_ID_UPG_COMPAT_CHECK, upgCtx);
    rsp = HdlrResp(::upgrade::SUCCESS, empty_str);
    return combine_responses(nicmgr_rsp, rsp);
}

//------------------------------------------------------------------------------
// handler to bring link(s) down
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::LinkDownHandler(UpgCtx& upgCtx)
{
    hal_ret_t                           ret;
    pd::pd_func_args_t                  pd_func_args = {0};
    pd::pd_uplink_tm_control_args_t     tm_args = {0};
    HdlrResp                            rsp, nicmgr_rsp;

    HAL_TRACE_DEBUG("[upgrade] Handling link down msg ...");

    // send TCP FIN on sessions with local EPs
    if ((hal::g_hal_cfg.features != hal::HAL_FEATURE_SET_GFT) &&
        (hal::g_hal_cfg.device_cfg.forwarding_mode != HAL_FORWARDING_MODE_CLASSIC)) {
        ret = session_handle_upgrade();
        if (ret != HAL_RET_OK) {
            HAL_TRACE_DEBUG("[upgrade] Session handle upgrade failed, err {}", ret);
            rsp = HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
            goto err;
        }
    }

    // disable all uplink ports
    // - as part of this delphi notifications will be sent out per port
    ret = linkmgr::port_disable(0);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("[upgrade] Port disable failed, err {}", ret);
        rsp = HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
        goto err;
    }

    // flush PB/TM for all uplinks
    tm_args.en      = false;
    tm_args.tm_port = TM_PORT_UPLINK_ALL;
    pd_func_args.pd_uplink_tm_control = &tm_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_UPLINK_TM_CONTROL,
                          &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("[upgrade] Unable to flush PB for uplinks. err: {}", ret);
        rsp = HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
        goto err;
    }

    rsp = HdlrResp(::upgrade::SUCCESS, empty_str);
err:
    nicmgr_rsp = upg_event_notify(MSG_ID_UPG_LINK_DOWN, upgCtx);
    return combine_responses(nicmgr_rsp, rsp);
}

//------------------------------------------------------------------------------
// handle Post link up
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::PostLinkUpHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling Post LinkUp msg ...");
    return upg_event_notify(MSG_ID_UPG_POST_LINK_UP, upgCtx);
}

//------------------------------------------------------------------------------
// handle Quiesce
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::ProcessQuiesceHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling quiesce msg ...");
    return upg_event_notify(MSG_ID_UPG_QUIESCE, upgCtx);
}


//------------------------------------------------------------------------------
// handle State save
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::SaveStateHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling save state msg ...");
    return upg_event_notify(MSG_ID_UPG_SAVE_STATE, upgCtx);
}

//------------------------------------------------------------------------------
// handle Upgrade failure
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::FailedHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling failed msg ...");
    return upg_event_notify(MSG_ID_UPG_FAIL, upgCtx);
}

//------------------------------------------------------------------------------
// handle Host down
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::HostDownHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling host down msg ...");
    return upg_event_notify(MSG_ID_UPG_HOSTDOWN, upgCtx);
}

//------------------------------------------------------------------------------
// handle Host up
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::HostUpHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling host up msg ...");
    return upg_event_notify(MSG_ID_UPG_HOSTUP, upgCtx);
}

//------------------------------------------------------------------------------
// handle post host down message by shutting of scheduler
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::PostHostDownHandler(UpgCtx& upgCtx)
{
    hal_ret_t ret = HAL_RET_OK;
    HdlrResp  rsp, nicmgr_rsp;

    HAL_TRACE_DEBUG("[upgrade] Handling post host down msg ...");

    // disable TX scheduler for all LIFs
    ret = lif_disable_tx_scheduler();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to disable TX scheduler for LIFs. err: {}", ret);
        rsp = HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
        goto err;
    }

    // quiesece the pipeline
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QUIESCE_START, NULL);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to quiesce. Failing ...");
        rsp = HdlrResp(::upgrade::INPROGRESS, HAL_RET_ENTRIES_str(ret));
        goto err;
        // return HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
    }

    // Reset tables
    ret = upgrade_table_reset();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to reset tables. err: {}", ret);
        rsp = HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
        goto err;
    }
    rsp = HdlrResp(::upgrade::SUCCESS, empty_str);
err:
    nicmgr_rsp = upg_event_notify(MSG_ID_UPG_POST_HOSTDOWN, upgCtx);
    return combine_responses(nicmgr_rsp, rsp);
}

//------------------------------------------------------------------------------
// take care post restart activities during upgrade
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::PostRestartHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling post restart msg ...");
    // TODO: mostly regular asic init path should work here, no special handling
    // needed
    return upg_event_notify(MSG_ID_UPG_POST_RESTART, upgCtx);
}

//------------------------------------------------------------------------------
// bring link up
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::LinkUpHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling link up msg ...");
    return upg_event_notify(MSG_ID_UPG_LINK_UP, upgCtx);
}

//------------------------------------------------------------------------------
// handle upgrade success by releasing any transient state we are holding
//------------------------------------------------------------------------------
void
upgrade_handler::SuccessHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling success msg ...");
    upg_event_notify(MSG_ID_UPG_SUCCESS, upgCtx);
}

//------------------------------------------------------------------------------
// handle upgrade abort
//------------------------------------------------------------------------------
void
upgrade_handler::AbortHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling abort msg ...");
    upg_event_notify(MSG_ID_UPG_ABORT, upgCtx);
}

}    // namespace upgrade
}    // namespace hal
