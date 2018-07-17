// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_post_state_handlers.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"

namespace upgrade {

using namespace std;

bool UpgPostStateHandler::PostUpgStateCompatCheckHandler(UpgCtx &ctx) {
    LogInfo("UpgPostStateHandler PostUpgStateCompatCheck returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStatePostBinRestartHandler(UpgCtx &ctx) {
    LogInfo("UpgPostStateHandler PostPostBinRestart returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateProcessQuiesceHandler(UpgCtx &ctx) {
    LogInfo("UpgPostStateHandler PostProcessesQuiesced returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateDataplaneDowntimePhase1Handler(UpgCtx &ctx) {
    LogInfo("UpgPostStateHandler PostDataplaneDowntimePhase1 returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateDataplaneDowntimePhase2Handler(UpgCtx &ctx) {
    LogInfo("UpgPostStateHandler PostDataplaneDowntimePhase2 returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateDataplaneDowntimePhase3Handler(UpgCtx &ctx) {
    LogInfo("UpgPostStateHandler PostDataplaneDowntimePhase3 returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateDataplaneDowntimePhase4Handler(UpgCtx &ctx) {
    LogInfo("UpgPostStateHandler PostDataplaneDowntimePhase4 returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateCleanupHandler(UpgCtx &ctx) {
    LogInfo("UpgPostStateHandler PostCleanup returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateSuccessHandler(UpgCtx &ctx) {
    LogInfo("UpgPostStateHandler PostSuccess returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateFailedHandler(UpgCtx &ctx) {
    LogInfo("UpgPostStateHandler PostFailed returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateAbortHandler(UpgCtx &ctx) {
    LogInfo("UpgPostStateHandler PostAborted returning");
    return true;
}

} // namespace upgrade
