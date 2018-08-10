// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_PRE_STATE_HANDLERS_H__
#define __UPGRADE_PRE_STATE_HANDLERS_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "upgrade_ctx.hpp"

namespace upgrade {

using namespace std;

class UpgPreStateHandler {
public:
    UpgPreStateHandler() {}
    bool PreUpgStateCompatCheckHandler(UpgCtx &ctx);
    bool PreUpgStatePostBinRestartHandler(UpgCtx &ctx);
    bool PreUpgStateProcessQuiesceHandler(UpgCtx &ctx);
    bool PreUpgStateDataplaneDowntimePhase1Handler(UpgCtx &ctx);
    bool PreUpgStateDataplaneDowntimePhase2Handler(UpgCtx &ctx);
    bool PreUpgStateDataplaneDowntimePhase3Handler(UpgCtx &ctx);
    bool PreUpgStateDataplaneDowntimePhase4Handler(UpgCtx &ctx);
    bool PreUpgStateCleanupHandler(UpgCtx &ctx);
    bool PreUpgStateSuccessHandler(UpgCtx &ctx);
    bool PreUpgStateFailedHandler(UpgCtx &ctx);
    bool PreUpgStateAbortHandler(UpgCtx &ctx);
    bool PreUpgStateLinkDownHandler(UpgCtx &ctx);
};
typedef std::shared_ptr<UpgPreStateHandler> UpgPreStateHandlerPtr;

} // namespace upgrade

#endif // __UPGRADE_PRE_STATE_HANDLERS_H__
