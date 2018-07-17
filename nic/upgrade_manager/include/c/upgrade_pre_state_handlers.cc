// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_pre_state_handlers.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/move_planner/lib/planner.h"

namespace upgrade {

using namespace std;

bool UpgPreStateHandler::PreUpgStateCompatCheckHandler(UpgCtx &ctx) {
    LogInfo("UpgPreStateHandler PrePreUpgState returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStatePostBinRestartHandler(UpgCtx &ctx) {
    LogInfo("UpgPreStateHandler PrePostBinRestart returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateProcessQuiesceHandler(UpgCtx &ctx) {
    LogInfo("UpgPreStateHandler PreProcessQuiesce returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateDataplaneDowntimePhase1Handler(UpgCtx &ctx) {
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

bool UpgPreStateHandler::PreUpgStateDataplaneDowntimePhase2Handler(UpgCtx &ctx) {
    LogInfo("UpgPreStateHandler PreDataplaneDowntimePhase2 returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateDataplaneDowntimePhase3Handler(UpgCtx &ctx) {
    LogInfo("UpgPreStateHandler PreDataplaneDowntimePhase3 returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateDataplaneDowntimePhase4Handler(UpgCtx &ctx) {
    LogInfo("UpgPreStateHandler PreDataplaneDowntimePhase4 returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateCleanupHandler(UpgCtx &ctx) {
    LogInfo("UpgPreStateHandler PreCleanup returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateSuccessHandler(UpgCtx &ctx) {
    LogInfo("UpgPreStateHandler PreUpgSuccess returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateFailedHandler(UpgCtx &ctx) {
    LogInfo("UpgPreStateHandler PreUpgFailed returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateAbortHandler(UpgCtx &ctx) {
    LogInfo("UpgPreStateHandler PreUpgAborted returning");
    return true;
}

} // namespace upgrade
