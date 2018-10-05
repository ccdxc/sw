//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "upgrade.hpp"
#include "nic/include/trace.hpp"
#include "nic/include/base.hpp"


namespace nicmgr {

namespace upgrade {

// Constructor method
nicmgr_upg_hndlr::nicmgr_upg_hndlr()
{
}

// Perform compat check
HdlrResp
nicmgr_upg_hndlr::CompatCheckHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    return resp;
}

// Bring link-down
HdlrResp
nicmgr_upg_hndlr::LinkDownHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    return resp;
}

// Bring data plane down
HdlrResp
nicmgr_upg_hndlr::DataplaneDowntimeStartHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    return resp;
}

// Post-binary restart handling
HdlrResp
nicmgr_upg_hndlr::PostRestartHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    return resp;
}

// Bring link-up
HdlrResp
nicmgr_upg_hndlr::LinkUpHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    return resp;
}

// Is system ready
HdlrResp
nicmgr_upg_hndlr::IsSystemReadyHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    return resp;
}

// Handle upgrade success
void
nicmgr_upg_hndlr::SuccessHandler(UpgCtx& upgCtx)
{
}

// Handle upgrade failure
void
nicmgr_upg_hndlr::FailedHandler(UpgCtx& upgCtx)
{
}

// Handle upgrade state cleanup
HdlrResp
nicmgr_upg_hndlr::CleanupHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    return resp;
}

// Handle upgrade abort
void
nicmgr_upg_hndlr::AbortHandler(UpgCtx& upgCtx)
{
}

} // namespace upgrade

} // namespace nicmgr
