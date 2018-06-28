// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_PRE_STATE_HANDLERS_H__
#define __UPGRADE_PRE_STATE_HANDLERS_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"

namespace upgrade {

using namespace std;

class UpgPreStateHandler {
public:
    UpgPreStateHandler() {}
    bool PreUpgReqRcvd();
    bool PrePreUpgState();
    bool PrePostBinRestart();
    bool PreProcessesQuiesced();
    bool PreDataplaneDowntimePhase1Start();
    bool PreDataplaneDowntimePhase2Start();
    bool PreDataplaneDowntimePhase3Start();
    bool PreDataplaneDowntimePhase4Start();
    bool PreCleanup();
    bool PreUpgSuccess();
    bool PreUpgFailed();
    bool PreUpgAborted();
};
typedef std::shared_ptr<UpgPreStateHandler> UpgPreStateHandlerPtr;

} // namespace upgrade

#endif // __UPGRADE_PRE_STATE_HANDLERS_H__
