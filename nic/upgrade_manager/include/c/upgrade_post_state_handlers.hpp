// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_POST_STATE_HANDLERS_H__
#define __UPGRADE_POST_STATE_HANDLERS_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"

namespace upgrade {

using namespace std;

class UpgPostStateHandler {
public:
    UpgPostStateHandler() {}
    bool PostUpgStateCompatCheckHandler();
    bool PostUpgStatePostBinRestartHandler();
    bool PostUpgStateProcessQuiesceHandler();
    bool PostUpgStateDataplaneDowntimePhase1Handler();
    bool PostUpgStateDataplaneDowntimePhase2Handler();
    bool PostUpgStateDataplaneDowntimePhase3Handler();
    bool PostUpgStateDataplaneDowntimePhase4Handler();
    bool PostUpgStateCleanupHandler();
    bool PostUpgStateSuccessHandler();
    bool PostUpgStateFailedHandler();
    bool PostUpgStateAbortHandler();
};
typedef std::shared_ptr<UpgPostStateHandler> UpgPostStateHandlerPtr;

} // namespace upgrade

#endif // __UPGRADE_POST_STATE_HANDLERS_H__
