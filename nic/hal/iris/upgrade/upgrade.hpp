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

class upgrade_handler : public UpgHandler {
public:
    upgrade_handler() {}
    HdlrResp CompatCheckHandler(UpgCtx& upgCtx);
    HdlrResp ProcessQuiesceHandler(UpgCtx& upgCtx);
    HdlrResp LinkDownHandler(UpgCtx& upgCtx);
    HdlrResp LinkUpHandler(UpgCtx& upgCtx);
    HdlrResp PostRestartHandler(UpgCtx& upgCtx);
    HdlrResp DataplaneDowntimeStartHandler(UpgCtx& upgCtx)
    void SuccessHandler(UpgCtx& upgCtx);
    void AbortHandler(UpgCtx& upgCtx);
};

} // namespace upgrade
} // namespace hal

#endif    // __HAL_UPG_HPP__
