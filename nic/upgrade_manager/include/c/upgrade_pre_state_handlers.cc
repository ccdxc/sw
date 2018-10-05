// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_pre_state_handlers.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/move_planner/lib/planner.h"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;

bool UpgPreStateHandler::PreCompatCheckHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PrePre returning");
    return true;
}

bool UpgPreStateHandler::PrePostBinRestartHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PrePostBinRestart returning");
    return true;
}

bool UpgPreStateHandler::PreProcessQuiesceHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PreProcessQuiesce returning");
    return true;
}

bool UpgPreStateHandler::PreLinkDownHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler Link Down returning");
    return true;
}

bool UpgPreStateHandler::PreLinkUpHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler Link Up returning");
    return true;
}

bool UpgPreStateHandler::PreDataplaneDowntimePhase1Handler(UpgCtx &ctx) {
    //TODO move to goto PostDataplaneDowntimePhase1
    if (planner::plan_and_move("/sw/nic/move_planner/hal_mem.json",
                               "/sw/nic/move_planner/hal_mem_after.json",
                               true) != planner::PLAN_SUCCESS) {
        UPG_LOG_DEBUG("UpgPreStateHandler PreDataplaneDowntimePhase1 Failed.");
        return false;
    }
    UPG_LOG_DEBUG("UpgPreStateHandler PreDataplaneDowntimePhase1 returning");
    return true;
}

bool UpgPreStateHandler::PreDataplaneDowntimePhase2Handler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PreDataplaneDowntimePhase2 returning");
    return true;
}

bool UpgPreStateHandler::PreDataplaneDowntimePhase3Handler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PreDataplaneDowntimePhase3 returning");
    return true;
}

bool UpgPreStateHandler::PreDataplaneDowntimePhase4Handler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PreDataplaneDowntimePhase4 returning");
    return true;
}

bool UpgPreStateHandler::PreDataplaneDowntimeStartHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PreDataplaneDowntimeStartHandler returning");
    return true;
}

bool UpgPreStateHandler::PreIsSystemReadyHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PreIsSystemReadyHandler returning");
    return true;
}

bool UpgPreStateHandler::PreCleanupHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PreCleanup returning");
    return true;
}

bool UpgPreStateHandler::PreSuccessHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PreUpgSuccess returning");
    return true;
}

bool UpgPreStateHandler::PreFailedHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PreUpgFailed returning");
    return true;
}

bool UpgPreStateHandler::PreAbortHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPreStateHandler PreUpgAborted returning");
    return true;
}

} // namespace upgrade
