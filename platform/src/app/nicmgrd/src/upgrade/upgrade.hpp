//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NICMGR_UPG_HPP__
#define __NICMGR_UPG_HPP__

#include "nic/upgrade_manager/export/upgcsdk/upgrade.hpp"

using upgrade::HdlrResp;
using upgrade::UpgCtx;
using upgrade::UpgHandler;

namespace nicmgr {

namespace upgrade {

class nicmgr_upg_hndlr : public UpgHandler {
public:
    nicmgr_upg_hndlr();
    //HdlrResp HandleUpgStateCompatCheck(UpgCtx& upgCtx);
    //HdlrResp HandleUpgStateLinkDown(UpgCtx& upgCtx);
    //HdlrResp HandleUpgStateDataplaneDowntimeStart(UpgCtx& upgCtx);
    //HdlrResp HandleUpgStatePostBinRestart(UpgCtx& upgCtx);
    //HdlrResp HandleUpgStateLinkUp(UpgCtx& upgCtx);
    //HdlrResp HandleUpgStateIsSystemReady(UpgCtx& upgCtx);
    //void HandleUpgStateSuccess(UpgCtx& upgCtx);
    //void HandleUpgStateFailed(UpgCtx& upgCtx);
    //HdlrResp HandleUpgStateCleanup(UpgCtx& upgCtx);
    //void HandleUpgStateAbort(UpgCtx& upgCtx);
};

} // namespace upgrade

} // namespace nicmgr
#endif    // __NICMGR_UPG_HPP__
