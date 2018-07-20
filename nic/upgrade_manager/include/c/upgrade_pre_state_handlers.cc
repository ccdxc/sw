// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_pre_state_handlers.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/move_planner/lib/planner.h"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;

bool UpgPreStateHandler::PreUpgStateCompatCheckHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PrePreUpgState returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStatePostBinRestartHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PrePostBinRestart returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateProcessQuiesceHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PreProcessQuiesce returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateDataplaneDowntimePhase1Handler(UpgCtx &ctx) {
    //TODO move to goto PostDataplaneDowntimePhase1
    if (ctx.upgType == UpgTypeDisruptive) {
        UPG_LOG_DEBUG("No need to plan for UpgTypeDisruptive");
        return true;
    }
    if (planner::plan_and_move("/sw/nic/move_planner/hal_mem.json",
                               "/sw/nic/move_planner/hal_mem_after.json",
                               true) != planner::PLAN_SUCCESS) {
        UPG_LOG_DEBUG("UpgPreStateHandler PreDataplaneDowntimePhase1 Failed.");
        return false;
    }
    UPG_LOG_DEBUG("UpgPreStateHandler PreDataplaneDowntimePhase1 returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateDataplaneDowntimePhase2Handler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PreDataplaneDowntimePhase2 returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateDataplaneDowntimePhase3Handler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PreDataplaneDowntimePhase3 returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateDataplaneDowntimePhase4Handler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PreDataplaneDowntimePhase4 returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateCleanupHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PreCleanup returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateSuccessHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PreUpgSuccess returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateFailedHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PreUpgFailed returning");
    return true;
}

bool UpgPreStateHandler::PreUpgStateAbortHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PreUpgAborted returning");
    return true;
}

} // namespace upgrade
