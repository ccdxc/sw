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

namespace hal {
namespace upgrade {

std::string empty_str("");

//------------------------------------------------------------------------------
// perform compat check
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::CompatCheckHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling compat checks msg ...");
    return HdlrResp(::upgrade::SUCCESS, empty_str);
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

    HAL_TRACE_DEBUG("[upgrade] Handling link down msg ...");

    // send TCP FIN on sessions with local EPs
    if ((hal::g_hal_cfg.features != hal::HAL_FEATURE_SET_GFT) &&
        (hal::g_hal_cfg.device_cfg.forwarding_mode != HAL_FORWARDING_MODE_CLASSIC)) {
        ret = session_handle_upgrade();
        if (ret != HAL_RET_OK) {
            return HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
        }
    }

    // disable all uplink ports
    // - as part of this delphi notifications will be sent out per port
    ret = linkmgr::port_disable(0);
    if (ret != HAL_RET_OK) {
        return HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
    }

    // flush PB/TM for all uplinks
    tm_args.en      = false;
    tm_args.tm_port = TM_PORT_UPLINK_ALL;
    pd_func_args.pd_uplink_tm_control = &tm_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_UPLINK_TM_CONTROL,
                          &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to flush PB for uplinks. err: {}", ret);
        return HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
    }

    return HdlrResp(::upgrade::SUCCESS, empty_str);
}

//------------------------------------------------------------------------------
// handle post host down message by shutting of scheduler
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::PostHostDownHandler(UpgCtx& upgCtx)
{
    hal_ret_t ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("[upgrade] Handling post host down msg ...");
    return HdlrResp(::upgrade::SUCCESS, empty_str);

    // disable TX scheduler for all LIFs
    ret = lif_disable_tx_scheduler();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to disable TX scheduler for LIFs. err: {}", ret);
        return HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
    }

    // quiesece the pipeline
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QUIESCE_START, NULL);
    if (ret != HAL_RET_OK) {
        return HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
    }
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QUIESCE_STOP, NULL);
    if (ret != HAL_RET_OK) {
        return HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
    }

    // Reset tables
    ret = upgrade_table_reset();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to reset tables. err: {}", ret);
        return HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
    }

    return HdlrResp(::upgrade::SUCCESS, empty_str);
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
    return HdlrResp(::upgrade::SUCCESS, empty_str);
}

//------------------------------------------------------------------------------
// bring link up
//------------------------------------------------------------------------------
HdlrResp
upgrade_handler::LinkUpHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling link up msg ...");
    return HdlrResp(::upgrade::SUCCESS, empty_str);
}

//------------------------------------------------------------------------------
// handle upgrade success by releasing any transient state we are holding
//------------------------------------------------------------------------------
void
upgrade_handler::SuccessHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling success msg ...");
}

//------------------------------------------------------------------------------
// handle upgrade abort
//------------------------------------------------------------------------------
void
upgrade_handler::AbortHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling abort msg ...");
}

}    // namespace upgrade
}    // namespace hal
