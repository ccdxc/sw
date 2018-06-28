// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_post_state_handlers.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"

namespace upgrade {

using namespace std;

bool UpgPostStateHandler::PostUpgReqRcvd(void) {
    LogInfo("UpgPostStateHandler PostUpgReqRcvd returning");
    return false;
}

bool UpgPostStateHandler::PostPreUpgState(void) {
    LogInfo("UpgPostStateHandler PostPreUpgState returning");
    return true;
}

bool UpgPostStateHandler::PostPostBinRestart(void) {
    LogInfo("UpgPostStateHandler PostPostBinRestart returning");
    return true;
}

bool UpgPostStateHandler::PostProcessesQuiesced(void) {
    LogInfo("UpgPostStateHandler PostProcessesQuiesced returning");
    return true;
}

bool UpgPostStateHandler::PostDataplaneDowntimePhase1Start(void) {
    LogInfo("UpgPostStateHandler PostDataplaneDowntimePhase1Start returning");
    return true;
}

bool UpgPostStateHandler::PostDataplaneDowntimeAdminQ(void) {
    LogInfo("UpgPostStateHandler PostDataplaneDowntimeAdminQ returning");
    return true;
}

bool UpgPostStateHandler::PostDataplaneDowntimePhase2Start(void) {
    LogInfo("UpgPostStateHandler PostDataplaneDowntimePhase2Start returning");
    return true;
}

bool UpgPostStateHandler::PostCleanup(void) {
    LogInfo("UpgPostStateHandler PostCleanup returning");
    return true;
}

bool UpgPostStateHandler::PostUpgSuccess(void) {
    LogInfo("UpgPostStateHandler PostUpgSuccess returning");
    return true;
}

bool UpgPostStateHandler::PostUpgFailed(void) {
    LogInfo("UpgPostStateHandler PostUpgFailed returning");
    return true;
}

bool UpgPostStateHandler::PostUpgAborted(void) {
    LogInfo("UpgPostStateHandler PostUpgAborted returning");
    return true;
}

} // namespace upgrade
