// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_pre_state_handlers.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/move_planner/lib/planner.h"

namespace upgrade {

using namespace std;

bool UpgPreStateHandler::PreUpgReqRcvd(void) {
    LogInfo("UpgPreStateHandler PreUpgReqRcvd returning");
    return true;
}

bool UpgPreStateHandler::PrePreUpgState(void) {
    LogInfo("UpgPreStateHandler PrePreUpgState returning");
    return true;
}

bool UpgPreStateHandler::PrePostBinRestart(void) {
    LogInfo("UpgPreStateHandler PrePostBinRestart returning");
    return true;
}

bool UpgPreStateHandler::PreProcessesQuiesced(void) {
    LogInfo("UpgPreStateHandler PreProcessesQuiesced returning");
    return true;
}

bool UpgPreStateHandler::PreDataplaneDowntimePhase1Start(void) {
    //TODO move to goto PostDataplaneDowntimePhase1Start
    if (planner::plan_and_move("/sw/nic/move_planner/hal_mem.json",
                               "/sw/nic/move_planner/hal_mem_after.json",
                               true) != planner::PLAN_FAIL) {
	LogInfo("UpgPreStateHandler PreDataplaneDowntimePhase1Start Failed.");
	return false;
    }
    LogInfo("UpgPreStateHandler PreDataplaneDowntimePhase1Start returning");
    return true;
}

bool UpgPreStateHandler::PreDataplaneDowntimePhase2Start(void) {
    LogInfo("UpgPreStateHandler PreDataplaneDowntimePhase2Start returning");
    return true;
}

bool UpgPreStateHandler::PreDataplaneDowntimePhase3Start(void) {
    LogInfo("UpgPreStateHandler PreDataplaneDowntimePhase3Start returning");
    return true;
}

bool UpgPreStateHandler::PreDataplaneDowntimePhase4Start(void) {
    LogInfo("UpgPreStateHandler PreDataplaneDowntimePhase4Start returning");
    return true;
}

bool UpgPreStateHandler::PreCleanup(void) {
    LogInfo("UpgPreStateHandler PreCleanup returning");
    return true;
}

bool UpgPreStateHandler::PreUpgSuccess(void) {
    LogInfo("UpgPreStateHandler PreUpgSuccess returning");
    return true;
}

bool UpgPreStateHandler::PreUpgFailed(void) {
    LogInfo("UpgPreStateHandler PreUpgFailed returning");
    return true;
}

bool UpgPreStateHandler::PreUpgAborted(void) {
    LogInfo("UpgPreStateHandler PreUpgAborted returning");
    return true;
}

} // namespace upgrade
