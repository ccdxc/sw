//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
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

std::string empty_string("");

//------------------------------------------------------------------------------
// perform compat check
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::CompatCheckHandler(UpgCtx& upgCtx)
{
    std::string preVer, postVer;
    HdlrResp    rsp;

    ::upgrade::UpgCtxApi::UpgCtxGetPreUpgTableVersion(upgCtx, ::upgrade::DEVCONFVER, preVer);
    ::upgrade::UpgCtxApi::UpgCtxGetPostUpgTableVersion(upgCtx, ::upgrade::DEVCONFVER, postVer);
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling compat checks msg ... preVer {} postVer {}", preVer, postVer);

    return upg_event_notify(MSG_ID_UPG_COMPAT_CHECK, upgCtx);
}

//------------------------------------------------------------------------------
// handler to bring link(s) down
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::LinkDownHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling link down msg ...");
    return upg_event_notify(MSG_ID_UPG_LINK_DOWN, upgCtx);
}

//------------------------------------------------------------------------------
// handle Post link up
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::PostLinkUpHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling Post LinkUp msg ...");
    return upg_event_notify(MSG_ID_UPG_POST_LINK_UP, upgCtx);
}

//------------------------------------------------------------------------------
// handle Quiesce
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::ProcessQuiesceHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling quiesce msg ...");
    return upg_event_notify(MSG_ID_UPG_QUIESCE, upgCtx);
}


//------------------------------------------------------------------------------
// handle State save
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::SaveStateHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling save state msg ...");
    return upg_event_notify(MSG_ID_UPG_SAVE_STATE, upgCtx);
}

//------------------------------------------------------------------------------
// handle Upgrade failure
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::FailedHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling failed msg ...");
    return upg_event_notify(MSG_ID_UPG_FAIL, upgCtx);
}

//------------------------------------------------------------------------------
// handle Host down
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::HostDownHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling host down msg ...");
    return upg_event_notify(MSG_ID_UPG_HOSTDOWN, upgCtx);
}

//------------------------------------------------------------------------------
// handle Host up
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::HostUpHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling host up msg ...");
    return upg_event_notify(MSG_ID_UPG_HOSTUP, upgCtx);
}

//------------------------------------------------------------------------------
// handle post host down message by shutting of scheduler
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::PostHostDownHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling post host down msg ...");
    return upg_event_notify(MSG_ID_UPG_POST_HOSTDOWN, upgCtx);
}

//------------------------------------------------------------------------------
// take care post restart activities during upgrade
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::PostRestartHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling post restart msg ...");
    // TODO: mostly regular asic init path should work here, no special handling
    // needed
    return upg_event_notify(MSG_ID_UPG_POST_RESTART, upgCtx);
}

//------------------------------------------------------------------------------
// bring link up
//------------------------------------------------------------------------------
HdlrResp
nicmgr_upgrade_handler::LinkUpHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling link up msg ...");
    return upg_event_notify(MSG_ID_UPG_LINK_UP, upgCtx);
}

//------------------------------------------------------------------------------
// handle upgrade success by releasing any transient state we are holding
//------------------------------------------------------------------------------
void
nicmgr_upgrade_handler::SuccessHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling success msg ...");
    upg_event_notify(MSG_ID_UPG_SUCCESS, upgCtx);
}

//------------------------------------------------------------------------------
// handle upgrade abort
//------------------------------------------------------------------------------
void
nicmgr_upgrade_handler::AbortHandler(UpgCtx& upgCtx)
{
    HAL_TRACE_DEBUG("[nicmgr_upgrade] Handling abort msg ...");
    upg_event_notify(MSG_ID_UPG_ABORT, upgCtx);
}

}    // namespace upgrade
}    // namespace hal
