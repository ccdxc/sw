// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_POST_STATE_HANDLERS_H__
#define __UPGRADE_POST_STATE_HANDLERS_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "upgrade_ctx.hpp"

namespace upgrade {

using namespace std;

class UpgPostStateHandler {
public:
    UpgPostStateHandler() {}
    bool PostCompatCheckHandler(UpgCtx &ctx);
    bool PostPostRestartHandler(UpgCtx &ctx);
    bool PostProcessQuiesceHandler(UpgCtx &ctx);
    bool PostLinkDownHandler(UpgCtx &ctx);
    bool PostLinkUpHandler(UpgCtx &ctx);
    bool PostDataplaneDowntimePhase1Handler(UpgCtx &ctx);
    bool PostDataplaneDowntimePhase2Handler(UpgCtx &ctx);
    bool PostDataplaneDowntimePhase3Handler(UpgCtx &ctx);
    bool PostDataplaneDowntimePhase4Handler(UpgCtx &ctx);
    bool PostSuccessHandler(UpgCtx &ctx);
    bool PostFailedHandler(UpgCtx &ctx);
    bool PostAbortHandler(UpgCtx &ctx);
    bool PostHostDowntHandler(UpgCtx &ctx);
    bool PostHostUpHandler(UpgCtx &ctx);
    bool PostPostHostDownHandler(UpgCtx &ctx);
    bool PostPostLinkUpHandler(UpgCtx &ctx);
    bool PostSaveStateHandler(UpgCtx &ctx);
};
typedef std::shared_ptr<UpgPostStateHandler> UpgPostStateHandlerPtr;

} // namespace upgrade

#endif // __UPGRADE_POST_STATE_HANDLERS_H__
