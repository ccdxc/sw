// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_pre_state_handlers.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"

namespace upgrade {

using namespace std;

void UpgPreStateHandler::PreUpgReqRcvd(void) {
    LogInfo("UpgPreStateHandler PreUpgReqRcvd returning");
    return;
}

void UpgPreStateHandler::PrePreUpgState(void) {
    LogInfo("UpgPreStateHandler PrePreUpgState returning");
    return;
}

void UpgPreStateHandler::PrePostBinRestart(void) {
    LogInfo("UpgPreStateHandler PrePostBinRestart returning");
    return;
}

void UpgPreStateHandler::PreProcessesQuiesced(void) {
    LogInfo("UpgPreStateHandler PreProcessesQuiesced returning");
    return;
}

void UpgPreStateHandler::PreDataplaneDowntimePhase1Start(void) {
    LogInfo("UpgPreStateHandler PreDataplaneDowntimePhase1Start returning");
    return;
}

void UpgPreStateHandler::PreDataplaneDowntimeAdminQ(void) {
    LogInfo("UpgPreStateHandler PreDataplaneDowntimeAdminQ returning");
    return;
}

void UpgPreStateHandler::PreDataplaneDowntimePhase2Start(void) {
    LogInfo("UpgPreStateHandler PreDataplaneDowntimePhase2Start returning");
    return;
}

void UpgPreStateHandler::PreCleanup(void) {
    LogInfo("UpgPreStateHandler PreCleanup returning");
    return;
}

void UpgPreStateHandler::PreUpgSuccess(void) {
    LogInfo("UpgPreStateHandler PreUpgSuccess returning");
    return;
}

void UpgPreStateHandler::PreUpgFailed(void) {
    LogInfo("UpgPreStateHandler PreUpgFailed returning");
    return;
}

void UpgPreStateHandler::PreUpgAborted(void) {
    LogInfo("UpgPreStateHandler PreUpgAborted returning");
    return;
}

} // namespace upgrade
