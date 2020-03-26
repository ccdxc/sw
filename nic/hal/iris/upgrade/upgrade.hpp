//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __HAL_UPGRADE_HPP__
#define __HAL_UPGRADE_HPP__

#include "nic/upgrade_manager/include/c/upgrade_ctx.hpp"
#include "nic/upgrade_manager/export/upgcsdk/upgrade_handler.hpp"
#include "nic/upgrade_manager/export/upgcsdk/upgrade_ctx_api.hpp"

using upgrade::HdlrResp;
using upgrade::UpgCtx;
using upgrade::UpgCtxApi;
using upgrade::UpgReqStateType;
using upgrade::UpgReqStateType::UpgStateCompatCheck;
using upgrade::UpgReqStateType::UpgStateProcessQuiesce;
using upgrade::UpgReqStateType::UpgStatePostRestart;
using upgrade::UpgReqStateType::UpgStateSuccess;
using upgrade::UpgReqStateType::UpgStateFailed;
using upgrade::UpgReqStateType::UpgStateAbort;
using upgrade::UpgReqStateType::UpgStateUpgPossible;
using upgrade::UpgReqStateType::UpgStateLinkDown;
using upgrade::UpgReqStateType::UpgStateLinkUp;
using upgrade::UpgReqStateType::UpgStateHostDown;
using upgrade::UpgReqStateType::UpgStateHostUp;
using upgrade::UpgReqStateType::UpgStatePostHostDown;
using upgrade::UpgReqStateType::UpgStatePostLinkUp;
using upgrade::UpgReqStateType::UpgStateSaveState;
using upgrade::UpgReqStateType::UpgStateTerminal;

namespace hal {
namespace upgrade {

class upgrade_handler : public ::upgrade::UpgHandler {
public:
    upgrade_handler() {}
    HdlrResp CompatCheckHandler(UpgCtx& upgCtx);
    HdlrResp ProcessQuiesceHandler(UpgCtx& upgCtx);
    HdlrResp LinkDownHandler(UpgCtx& upgCtx);
    HdlrResp HostDownHandler(UpgCtx& upgCtx);
    HdlrResp PostHostDownHandler(UpgCtx& upgCtx);
    HdlrResp PostRestartHandler(UpgCtx& upgCtx);
    HdlrResp SaveStateHandler(UpgCtx& upgCtx);
    HdlrResp LinkUpHandler(UpgCtx& upgCtx);
    HdlrResp HostUpHandler(UpgCtx& upgCtx);
    HdlrResp PostLinkUpHandler(UpgCtx& upgCtx);
    void SuccessHandler(UpgCtx& upgCtx);
    void AbortHandler(UpgCtx& upgCtx);
    HdlrResp FailedHandler(UpgCtx& upgCtx);
};

}    // namespace upgrade
}    // namespace hal

#endif    // __HAL_UPGRADE_HPP__

