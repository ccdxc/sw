//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/iris/upgrade/upgrade.hpp"
#include "nic/include/trace.hpp"
#include "nic/include/base.hpp"

namespace hal {

namespace upgrade {

//------------------------------------------------------------------------------
// constructor method
//------------------------------------------------------------------------------
hal_upg_hndlr::hal_upg_hndlr()
{
}

//------------------------------------------------------------------------------
// perform compat check
//------------------------------------------------------------------------------
HdlrResp
hal_upg_hndlr::CompatCheckHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling compat checks msg ...");
    return HdlrResp(SUCCESS, "");
}

//------------------------------------------------------------------------------
// perform quiesce start
//------------------------------------------------------------------------------
HdlrResp
hal_upg_hndlr::ProcessQuiesceHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling queiesce msg ...");
    return HdlrResp(SUCCESS, "");
}

//------------------------------------------------------------------------------
// bring link down
//------------------------------------------------------------------------------
HdlrResp
hal_upg_hndlr::LinkDownHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    HAL_TRACE_DEBUG("[upgrade] Handling link down msg ...");
    return resp;
}

//------------------------------------------------------------------------------
// bring link up
//------------------------------------------------------------------------------
HdlrResp
hal_upg_hndlr::LinkUpHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    HAL_TRACE_DEBUG("[upgrade] Handling link up msg ...");
    return resp;
}

//------------------------------------------------------------------------------
// post restart handling
//------------------------------------------------------------------------------
HdlrResp
hal_upg_hndlr::PostRestartHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    HAL_TRACE_DEBUG("[upgrade] Handling post restart msg ...");
    return resp;
}

//------------------------------------------------------------------------------
// bring data plane down
//------------------------------------------------------------------------------
HdlrResp
hal_upg_hndlr::DataplaneDowntimeStartHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    HAL_TRACE_DEBUG("[upgrade] Handling dataplane down msg ...");
    return resp;
}

//------------------------------------------------------------------------------
// handle upgrade success by releasing any transient state we are holding
//------------------------------------------------------------------------------
void
hal_upg_hndlr::SuccessHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling success msg ...");
}

//------------------------------------------------------------------------------
// handle upgrade abort
//------------------------------------------------------------------------------
void
hal_upg_hndlr::AbortHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[upgrade] Handling abort msg ...");
}

} // namespace upgrade

} // namespace hal
