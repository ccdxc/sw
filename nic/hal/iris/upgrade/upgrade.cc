//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/iris/upgrade/upgrade.hpp"
#include "nic/include/trace.hpp"
#include "nic/include/base.hpp"

namespace hal {

namespace upgrade {

// Constructor method
hal_upg_hndlr::hal_upg_hndlr()
{
    HAL_TRACE_DEBUG("Placeholder");
}

// Perform compat check
HdlrResp
hal_upg_hndlr::HandleUpgStateCompatCheck(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    HAL_TRACE_DEBUG("Placeholder");
    return resp;
}

// Bring link-down
HdlrResp
hal_upg_hndlr::HandleUpgStateLinkDown(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    HAL_TRACE_DEBUG("Placeholder");
    return resp;
}

// Bring data plane down
HdlrResp
hal_upg_hndlr::HandleUpgStateDataplaneDowntimeStart(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    HAL_TRACE_DEBUG("Placeholder");
    return resp;
}

// Post-binary restart handling
HdlrResp
hal_upg_hndlr::HandleUpgStatePostBinRestart(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    HAL_TRACE_DEBUG("Placeholder");
    return resp;
}

// Bring link-up
HdlrResp
hal_upg_hndlr::HandleUpgStateLinkUp(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    HAL_TRACE_DEBUG("Placeholder");
    return resp;
}

// Is system ready
HdlrResp
hal_upg_hndlr::HandleUpgStateIsSystemReady(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    HAL_TRACE_DEBUG("Placeholder");
    return resp;
}

// Handle upgrade success
void
hal_upg_hndlr::HandleUpgStateSuccess(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("Placeholder");
}

// Handle upgrade failure
void
hal_upg_hndlr::HandleUpgStateFailed(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("Placeholder");
}

// Handle upgrade state cleanup
HdlrResp
hal_upg_hndlr::HandleUpgStateCleanup(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    HAL_TRACE_DEBUG("Placeholder");
    return resp;
}

// Handle upgrade abort
void
hal_upg_hndlr::HandleUpgStateAbort(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("Placeholder");
}

} // namespace upgrade

} // namespace hal
