// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_pre_state_handlers.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/move_planner/lib/planner.h"

namespace upgrade {

using namespace std;

bool UpgPreStateHandler::PreUpgStateCompatCheckHandler(void) {
    LogInfo("UpgPreStateHandler PrePreUpgState returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStatePostBinRestartHandler(void) {
    LogInfo("UpgPreStateHandler PrePostBinRestart returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateProcessQuiesceHandler(void) {
    LogInfo("UpgPreStateHandler PreProcessQuiesce returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateDataplaneDowntimePhase1Handler(void) {
    //TODO move to goto PostDataplaneDowntimePhase1
    if (planner::plan_and_move("/sw/nic/move_planner/hal_mem.json",
                               "/sw/nic/move_planner/hal_mem_after.json",
                               true) != planner::PLAN_SUCCESS) {
        LogInfo("UpgPreStateHandler PreDataplaneDowntimePhase1 Failed.");
        return false;
    }
    LogInfo("UpgPreStateHandler PreDataplaneDowntimePhase1 returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateDataplaneDowntimePhase2Handler(void) {
    LogInfo("UpgPreStateHandler PreDataplaneDowntimePhase2 returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateDataplaneDowntimePhase3Handler(void) {
    LogInfo("UpgPreStateHandler PreDataplaneDowntimePhase3 returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateDataplaneDowntimePhase4Handler(void) {
    LogInfo("UpgPreStateHandler PreDataplaneDowntimePhase4 returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateCleanupHandler(void) {
    LogInfo("UpgPreStateHandler PreCleanup returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateSuccessHandler(void) {
    LogInfo("UpgPreStateHandler PreUpgSuccess returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateFailedHandler(void) {
    LogInfo("UpgPreStateHandler PreUpgFailed returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateAbortHandler(void) {
    LogInfo("UpgPreStateHandler PreUpgAborted returning");
    return true;
}

} // namespace upgrade
