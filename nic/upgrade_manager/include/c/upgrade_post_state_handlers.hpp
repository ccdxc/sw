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
    bool PostUpgStateCompatCheckHandler(UpgCtx &ctx);
    bool PostUpgStatePostBinRestartHandler(UpgCtx &ctx);
    bool PostUpgStateProcessQuiesceHandler(UpgCtx &ctx);
    bool PostUpgStateDataplaneDowntimePhase1Handler(UpgCtx &ctx);
    bool PostUpgStateDataplaneDowntimePhase2Handler(UpgCtx &ctx);
    bool PostUpgStateDataplaneDowntimePhase3Handler(UpgCtx &ctx);
    bool PostUpgStateDataplaneDowntimePhase4Handler(UpgCtx &ctx);
    bool PostUpgStateCleanupHandler(UpgCtx &ctx);
    bool PostUpgStateSuccessHandler(UpgCtx &ctx);
    bool PostUpgStateFailedHandler(UpgCtx &ctx);
    bool PostUpgStateAbortHandler(UpgCtx &ctx);
    bool PostUpgStateLinkDownHandler(UpgCtx &ctx);
};
typedef std::shared_ptr<UpgPostStateHandler> UpgPostStateHandlerPtr;

} // namespace upgrade

#endif // __UPGRADE_POST_STATE_HANDLERS_H__
