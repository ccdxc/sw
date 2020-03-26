//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/iris/upgrade/upgrade.hpp"
#include "nic/hal/iris/upgrade/nicmgr_upgrade.hpp"
#include "nic/include/base.hpp"
#include "nic/hal/hal_trace.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/linkmgr/linkmgr.hpp"
#include "nic/hal/plugins/cfg/lif/lif.hpp"
#include "nic/hal/src/internal/system.hpp"
#include "nic/hal/iris/upgrade/upg_ipc.hpp"

namespace hal {
namespace upgrade {

std::string empty_string("");

//------------------------------------------------------------------------------
// perform compat check
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::CompatCheckHandler(UpgCtx& upgCtx)
{
    std::string preVer, postVer;
    HdlrResp    rsp;
    uint32_t  prevstate = prevExecState;

    ::upgrade::UpgCtxApi::UpgCtxGetPreUpgTableVersion(upgCtx, ::upgrade::DEVCONFVER, preVer);
    ::upgrade::UpgCtxApi::UpgCtxGetPostUpgTableVersion(upgCtx, ::upgrade::DEVCONFVER, postVer);
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling compat checks msg ... preVer {} postVer {}", preVer, postVer);
    prevExecState = UpgStateCompatCheck;

    return upg_event_notify(MSG_ID_UPG_COMPAT_CHECK, upgCtx, prevstate);
}

//------------------------------------------------------------------------------
// handler to bring link(s) down
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::LinkDownHandler(UpgCtx& upgCtx)
{
    uint32_t  prevstate = prevExecState;
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling link down msg ...");
    prevExecState = UpgStateLinkDown;
    return upg_event_notify(MSG_ID_UPG_LINK_DOWN, upgCtx, prevstate);
}

//------------------------------------------------------------------------------
// handle Post link up
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::PostLinkUpHandler(UpgCtx& upgCtx)
{
    uint32_t  prevstate = prevExecState;
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling Post LinkUp msg ...");
    prevExecState = UpgStatePostLinkUp;
    return upg_event_notify(MSG_ID_UPG_POST_LINK_UP, upgCtx, prevstate);
}

//------------------------------------------------------------------------------
// handle Quiesce
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::ProcessQuiesceHandler(UpgCtx& upgCtx)
{
    uint32_t  prevstate = prevExecState;
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling quiesce msg ...");
    prevExecState = UpgStateProcessQuiesce;
    return upg_event_notify(MSG_ID_UPG_QUIESCE, upgCtx, prevstate);
}


//------------------------------------------------------------------------------
// handle State save
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::SaveStateHandler(UpgCtx& upgCtx)
{
    uint32_t  prevstate = prevExecState;
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling save state msg ...");
    prevExecState = UpgStateSaveState;
    return upg_event_notify(MSG_ID_UPG_SAVE_STATE, upgCtx, prevstate);
}

//------------------------------------------------------------------------------
// handle Upgrade failure
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::FailedHandler(UpgCtx& upgCtx)
{
    uint32_t  prevstate = prevExecState;
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling failed msg ...");
    prevExecState = UpgStateFailed;
    return upg_event_notify(MSG_ID_UPG_FAIL, upgCtx, prevstate);
}

//------------------------------------------------------------------------------
// handle Host down
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::HostDownHandler(UpgCtx& upgCtx)
{
    uint32_t  prevstate = prevExecState;
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling host down msg ...");
    prevExecState = UpgStateHostDown;
    return upg_event_notify(MSG_ID_UPG_HOSTDOWN, upgCtx, prevstate);
}

//------------------------------------------------------------------------------
// handle Host up
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::HostUpHandler(UpgCtx& upgCtx)
{
    uint32_t  prevstate = prevExecState;
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling host up msg ...");
    prevExecState = UpgStateHostUp;
    return upg_event_notify(MSG_ID_UPG_HOSTUP, upgCtx, prevstate);
}

//------------------------------------------------------------------------------
// handle post host down message by shutting of scheduler
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::PostHostDownHandler(UpgCtx& upgCtx)
{
    uint32_t  prevstate = prevExecState;
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling post host down msg ...");
    prevExecState = UpgStatePostHostDown;
    return upg_event_notify(MSG_ID_UPG_POST_HOSTDOWN, upgCtx, prevstate);
}

//------------------------------------------------------------------------------
// take care post restart activities during upgrade
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::PostRestartHandler(UpgCtx& upgCtx)
{
    uint32_t  prevstate = prevExecState;
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling post restart msg ...");
    // TODO: mostly regular asic init path should work here, no special handling
    // needed
    prevExecState = UpgStatePostRestart;
    return upg_event_notify(MSG_ID_UPG_POST_RESTART, upgCtx, prevstate);
}

//------------------------------------------------------------------------------
// bring link up
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::LinkUpHandler(UpgCtx& upgCtx)
{
    uint32_t  prevstate = prevExecState;
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling link up msg ...");
    prevExecState = UpgStateLinkUp;
    return upg_event_notify(MSG_ID_UPG_LINK_UP, upgCtx, prevstate);
}

//------------------------------------------------------------------------------
// handle upgrade success by releasing any transient state we are holding
//------------------------------------------------------------------------------
void
nicmgr_upgrade_handler::SuccessHandler(UpgCtx& upgCtx)
{
    uint32_t  prevstate = prevExecState;
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling success msg ...");
    prevExecState = UpgStateSuccess;
    upg_event_notify(MSG_ID_UPG_SUCCESS, upgCtx, prevstate);
}

//------------------------------------------------------------------------------
// handle upgrade abort
//------------------------------------------------------------------------------
void
nicmgr_upgrade_handler::AbortHandler(UpgCtx& upgCtx)
{
    uint32_t  prevstate = prevExecState;
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling abort msg ...");
    prevExecState = UpgStateAbort;
    upg_event_notify(MSG_ID_UPG_ABORT, upgCtx, prevstate);
}

}    // namespace upgrade
}    // namespace hal
