// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_PRE_STATE_HANDLERS_H__
#define __UPGRADE_PRE_STATE_HANDLERS_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"

namespace upgrade {

using namespace std;

class UpgPreStateHandler {
public:
    UpgPreStateHandler() {}
    void PreUpgReqRcvd();
    void PrePreUpgState();
    void PrePostBinRestart();
    void PreProcessesQuiesced();
    void PreDataplaneDowntimePhase1Start();
    void PreDataplaneDowntimeAdminQ();
    void PreDataplaneDowntimePhase2Start();
    void PreCleanup();
    void PreUpgSuccess();
    void PreUpgFailed();
    void PreUpgAborted();
};
typedef std::shared_ptr<UpgPreStateHandler> UpgPreStateHandlerPtr;

} // namespace upgrade

#endif // __UPGRADE_PRE_STATE_HANDLERS_H__
