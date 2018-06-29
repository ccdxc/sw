// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_PRE_STATE_HANDLERS_H__
#define __UPGRADE_PRE_STATE_HANDLERS_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"

namespace upgrade {

using namespace std;

class UpgPreStateHandler {
public:
    UpgPreStateHandler() {}
    bool PreUpgStateCompatCheckHandler();
    bool PreUpgStatePostBinRestartHandler();
    bool PreUpgStateProcessQuiesceHandler();
    bool PreUpgStateDataplaneDowntimePhase1Handler();
    bool PreUpgStateDataplaneDowntimePhase2Handler();
    bool PreUpgStateDataplaneDowntimePhase3Handler();
    bool PreUpgStateDataplaneDowntimePhase4Handler();
    bool PreUpgStateCleanupHandler();
    bool PreUpgStateSuccessHandler();
    bool PreUpgStateFailedHandler();
    bool PreUpgStateAbortHandler();
};
typedef std::shared_ptr<UpgPreStateHandler> UpgPreStateHandlerPtr;

} // namespace upgrade

#endif // __UPGRADE_PRE_STATE_HANDLERS_H__
