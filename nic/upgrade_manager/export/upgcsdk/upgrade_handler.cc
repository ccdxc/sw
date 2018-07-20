// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_handler.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;

HdlrResp UpgHandler::HandleUpgStateCompatCheck(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler HandleUpgStateCompatCheck not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleUpgStatePostBinRestart(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler HandleUpgStatePostBinRestart not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleUpgStateProcessQuiesce(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler HandleUpgStateProcessesQuiesced not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleUpgStateDataplaneDowntimePhase1(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler HandleUpgStateDataplaneDowntimePhase1 not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleUpgStateDataplaneDowntimePhase2(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler HandleUpgStateDataplaneDowntimePhase2 not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleUpgStateDataplaneDowntimePhase3(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler HandleUpgStateDataplaneDowntimePhase3 not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleUpgStateDataplaneDowntimePhase4(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler HandleUpgStateDataplaneDowntimePhase4 not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleUpgStateCleanup(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler HandleUpgStateCleanup not implemented by service");
    return resp;
}

void UpgHandler::HandleUpgStateSuccess(UpgCtx& upgCtx) {
    UPG_LOG_DEBUG("UpgHandler HandleUpgStateSuccess not implemented by service");
    return;
}

void UpgHandler::HandleUpgStateFailed(UpgCtx& upgCtx) {
    UPG_LOG_DEBUG("UpgHandler HandleUpgStateFailed not implemented by service");
    return;
}

void UpgHandler::HandleUpgStateAbort(UpgCtx& upgCtx) {
    UPG_LOG_DEBUG("UpgHandler HandleUpgStateAbort not implemented by service");
    return;
}

} // namespace upgrade
