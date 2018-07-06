// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_handler.hpp"

namespace upgrade {

using namespace std;

HdlrResp UpgHandler::HandleUpgStateCompatCheck(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleUpgStateCompatCheck not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleUpgStatePostBinRestart(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleUpgStatePostBinRestart not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleUpgStateProcessQuiesce(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleUpgStateProcessesQuiesced not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleUpgStateDataplaneDowntimePhase1(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleUpgStateDataplaneDowntimePhase1 not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleUpgStateDataplaneDowntimePhase2(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleUpgStateDataplaneDowntimePhase2 not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleUpgStateDataplaneDowntimePhase3(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleUpgStateDataplaneDowntimePhase3 not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleUpgStateDataplaneDowntimePhase4(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleUpgStateDataplaneDowntimePhase4 not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleUpgStateCleanup(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleUpgStateCleanup not implemented by service");
    return resp;
}

void UpgHandler::HandleUpgStateSuccess(UpgCtx& upgCtx) {
    LogInfo("UpgHandler HandleUpgStateSuccess not implemented by service");
    return;
}

void UpgHandler::HandleUpgStateFailed(UpgCtx& upgCtx) {
    LogInfo("UpgHandler HandleUpgStateFailed not implemented by service");
    return;
}

void UpgHandler::HandleUpgStateAbort(UpgCtx& upgCtx) {
    LogInfo("UpgHandler HandleUpgStateAbort not implemented by service");
    return;
}

} // namespace upgrade
