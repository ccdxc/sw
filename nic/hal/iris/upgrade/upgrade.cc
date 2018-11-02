//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/iris/upgrade/upgrade.hpp"
#include "nic/include/base.hpp"
#include "nic/hal/hal_trace.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/linkmgr/linkmgr.hpp"

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
    hal_ret_t    ret;

    HAL_TRACE_DEBUG("[upgrade] Handling link down msg ...");

    // disable all uplink ports and as part of this delphi notifications
    // will be sent out
    ret = linkmgr::port_disable(0);
    if (ret != HAL_RET_OK) {
        return HdlrResp(::upgrade::FAIL, HAL_RET_ENTRIES_str(ret));
    }

    // quiesece the pipeline (TODO: only for uplink ports !!)
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QUIESCE_START, NULL);
    if (ret != HAL_RET_OK) {
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
    HAL_TRACE_DEBUG("[upgrade] Handling post host down msg ...");
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
    HdlrResp resp = {};
    HAL_TRACE_DEBUG("[upgrade] Handling link up msg ...");
    return resp;
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
