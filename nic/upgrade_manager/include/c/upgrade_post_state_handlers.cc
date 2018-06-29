// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_post_state_handlers.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"

namespace upgrade {

using namespace std;

bool UpgPostStateHandler::PostUpgStateCompatCheckHandler(void) {
    LogInfo("UpgPostStateHandler PostUpgStateCompatCheck returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStatePostBinRestartHandler(void) {
    LogInfo("UpgPostStateHandler PostPostBinRestart returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateProcessQuiesceHandler(void) {
    LogInfo("UpgPostStateHandler PostProcessesQuiesced returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateDataplaneDowntimePhase1Handler(void) {
    LogInfo("UpgPostStateHandler PostDataplaneDowntimePhase1 returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateDataplaneDowntimePhase2Handler(void) {
    LogInfo("UpgPostStateHandler PostDataplaneDowntimePhase2 returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateDataplaneDowntimePhase3Handler(void) {
    LogInfo("UpgPostStateHandler PostDataplaneDowntimePhase3 returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateDataplaneDowntimePhase4Handler(void) {
    LogInfo("UpgPostStateHandler PostDataplaneDowntimePhase4 returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateCleanupHandler(void) {
    LogInfo("UpgPostStateHandler PostCleanup returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateSuccessHandler(void) {
    LogInfo("UpgPostStateHandler PostSuccess returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateFailedHandler(void) {
    LogInfo("UpgPostStateHandler PostFailed returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateAbortHandler(void) {
    LogInfo("UpgPostStateHandler PostAborted returning");
    return true;
}

} // namespace upgrade
