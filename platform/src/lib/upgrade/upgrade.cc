//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nicmgr_upgrade.hpp"
#include "nicmgr_delphi_client.hpp"
#include "nic/include/trace.hpp"
#include "nic/include/base.hpp"
#include "dev.hpp"
#include "eth_dev.hpp"
#include "nicmgr.pb.h"


using namespace upgrade;
using namespace nicmgr;

namespace nicmgr {

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

HdlrResp 
nicmgr_upg_hndlr::ProcessQuiesceHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    return resp;
}

// Bring link-down
HdlrResp
nicmgr_upg_hndlr::LinkDownHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {};
    return resp;
}

HdlrResp 
nicmgr_upg_hndlr::PostHostDownHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
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

// Handle upgrade abort
void
nicmgr_upg_hndlr::AbortHandler(UpgCtx& upgCtx)
{
}

HdlrResp 
nicmgr_upg_hndlr::SaveStateHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    return resp;
}

HdlrResp 
nicmgr_upg_hndlr::PostLinkUpHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    return resp;
}

HdlrResp 
nicmgr_upg_hndlr::HostDownHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    return resp;
}

HdlrResp 
nicmgr_upg_hndlr::HostUpHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    return resp;
}


} // namespace nicmgr
