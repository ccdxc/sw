// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_POST_STATE_HANDLERS_H__
#define __UPGRADE_POST_STATE_HANDLERS_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"

namespace upgrade {

using namespace std;

class UpgPostStateHandler {
public:
    UpgPostStateHandler() {}
    bool PostUpgReqRcvd();
    bool PostPreUpgState();
    bool PostPostBinRestart();
    bool PostProcessesQuiesced();
    bool PostDataplaneDowntimePhase1Start();
    bool PostDataplaneDowntimePhase2Start();
    bool PostDataplaneDowntimePhase3Start();
    bool PostDataplaneDowntimePhase4Start();
    bool PostCleanup();
    bool PostUpgSuccess();
    bool PostUpgFailed();
    bool PostUpgAborted();
};
typedef std::shared_ptr<UpgPostStateHandler> UpgPostStateHandlerPtr;

} // namespace upgrade

#endif // __UPGRADE_POST_STATE_HANDLERS_H__
