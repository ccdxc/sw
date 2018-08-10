// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_post_state_handlers.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;

bool UpgPostStateHandler::PostUpgStateCompatCheckHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPostStateHandler PostUpgStateCompatCheck returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStatePostBinRestartHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPostStateHandler PostPostBinRestart returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateProcessQuiesceHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPostStateHandler PostProcessesQuiesced returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateLinkDownHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPostStateHandler Link Down returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateDataplaneDowntimePhase1Handler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPostStateHandler PostDataplaneDowntimePhase1 returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateDataplaneDowntimePhase2Handler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPostStateHandler PostDataplaneDowntimePhase2 returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateDataplaneDowntimePhase3Handler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPostStateHandler PostDataplaneDowntimePhase3 returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateDataplaneDowntimePhase4Handler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPostStateHandler PostDataplaneDowntimePhase4 returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateCleanupHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPostStateHandler PostCleanup returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateSuccessHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPostStateHandler PostSuccess returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateFailedHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPostStateHandler PostFailed returning");
    return true;
}

bool UpgPostStateHandler::PostUpgStateAbortHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPostStateHandler PostAborted returning");
    return true;
}

} // namespace upgrade
