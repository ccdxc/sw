//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/iris/upgrade/upgrade.hpp"
#include "nic/include/base.hpp"
#include "nic/include/trace.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/linkmgr/linkmgr.hpp"

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
    hal_ret_t    ret;

    HAL_TRACE_DEBUG("[upgrade] Handling queiesce msg ...");
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QUIESCE_START, NULL);
    if (ret != HAL_RET_OK) {
        return HdlrResp(FAIL, std::string(HAL_RET_ENTRIES_str));
    }
    return HdlrResp(SUCCESS, "");
}

//------------------------------------------------------------------------------
// bring link down
//------------------------------------------------------------------------------
HdlrResp
hal_upg_hndlr::LinkDownHandler(UpgCtx& upgCtx)
{
    hal_ret_t    ret;

    HAL_TRACE_DEBUG("[upgrade] Handling link down msg ...");
    ret = linkmgr::port_disable(0);
    if (ret != HAL_RET_OK) {
        return HdlrResp(FAIL, std::string(HAL_RET_ENTRIES_str))
    }
    return HdlrResp(SUCCESS, "");
}

#if 0
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
#endif

}    // namespace upgrade
}    // namespace hal
