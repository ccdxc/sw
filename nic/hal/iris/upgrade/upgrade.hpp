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

