//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NICMGR_UPG_HPP__
#define __NICMGR_UPG_HPP__

#include "nic/upgrade_manager/export/upgcsdk/upgrade.hpp"
#include "nic/upgrade_manager/export/upgcsdk/upgrade_handler.hpp"

using namespace upgrade;

using upgrade::HdlrResp;
using upgrade::UpgCtx;
using upgrade::UpgHandler;

namespace nicmgr {


class nicmgr_upg_hndlr : public UpgHandler {
public:
    nicmgr_upg_hndlr();
    HdlrResp CompatCheckHandler(UpgCtx& upgCtx);
    HdlrResp ProcessQuiesceHandler(UpgCtx& upgCtx);
    HdlrResp LinkDownHandler(UpgCtx& upgCtx);
    HdlrResp PostRestartHandler(UpgCtx& upgCtx);
    HdlrResp HostDownHandler(UpgCtx& upgCtx);
    HdlrResp PostHostDownHandler(UpgCtx& upgCtx);
    HdlrResp SaveStateHandler(UpgCtx& upgCtx);
    HdlrResp HostUpHandler(UpgCtx& upgCtx);
    HdlrResp LinkUpHandler(UpgCtx& upgCtx);
    HdlrResp PostLinkUpHandler(UpgCtx& upgCtx);
    void SuccessHandler(UpgCtx& upgCtx);
    void FailedHandler(UpgCtx& upgCtx);
    void AbortHandler(UpgCtx& upgCtx);
};

} // namespace nicmgr
#endif    // __NICMGR_UPG_HPP__
