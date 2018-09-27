//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __HAL_UPG_HPP__
#define __HAL_UPG_HPP__

#include "nic/upgrade_manager/include/c/upgrade_ctx.hpp"
#include "nic/upgrade_manager/export/upgcsdk/upgrade_handler.hpp"

using upgrade::HdlrResp;
using upgrade::UpgCtx;
using upgrade::UpgHandler;

namespace hal {
namespace upgrade {

class hal_upg_hndlr : public UpgHandler {
public:
    hal_upg_hndlr();
    HdlrResp HandleUpgStateCompatCheck(UpgCtx& upgCtx);
    HdlrResp HandleUpgStateLinkDown(UpgCtx& upgCtx);
    HdlrResp HandleUpgStateDataplaneDowntimeStart(UpgCtx& upgCtx);
    HdlrResp HandleUpgStatePostBinRestart(UpgCtx& upgCtx);
    HdlrResp HandleUpgStateLinkUp(UpgCtx& upgCtx);
    HdlrResp HandleUpgStateIsSystemReady(UpgCtx& upgCtx);
    void HandleUpgStateSuccess(UpgCtx& upgCtx);
    void HandleUpgStateFailed(UpgCtx& upgCtx);
    HdlrResp HandleUpgStateCleanup(UpgCtx& upgCtx);
    void HandleUpgStateAbort(UpgCtx& upgCtx);
};

} // namespace upgrade
} // namespace hal

#endif    // __HAL_UPG_HPP__
