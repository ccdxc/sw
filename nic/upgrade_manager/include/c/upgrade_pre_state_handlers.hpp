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
    bool PreCompatCheckHandler(UpgCtx &ctx);
    bool PrePostRestartHandler(UpgCtx &ctx);
    bool PreProcessQuiesceHandler(UpgCtx &ctx);
    bool PreDataplaneDowntimePhase1Handler(UpgCtx &ctx);
    bool PreDataplaneDowntimePhase2Handler(UpgCtx &ctx);
    bool PreDataplaneDowntimePhase3Handler(UpgCtx &ctx);
    bool PreDataplaneDowntimePhase4Handler(UpgCtx &ctx);
    bool PreSuccessHandler(UpgCtx &ctx);
    bool PreFailedHandler(UpgCtx &ctx);
    bool PreAbortHandler(UpgCtx &ctx);
    bool PreLinkDownHandler(UpgCtx &ctx);
    bool PreLinkUpHandler(UpgCtx &ctx);
    bool PreHostDownHandler(UpgCtx &ctx);
    bool PreHostUpHandler(UpgCtx &ctx);
    bool PrePostHostDownHandler(UpgCtx &ctx);
    bool PrePostLinkUpHandler(UpgCtx &ctx);
    bool PreSaveStateHandler(UpgCtx &ctx);
private:
    bool ImageCompatCheck(UpgCtx &ctx);
};
typedef std::shared_ptr<UpgPreStateHandler> UpgPreStateHandlerPtr;

} // namespace upgrade

#endif // __UPGRADE_PRE_STATE_HANDLERS_H__
