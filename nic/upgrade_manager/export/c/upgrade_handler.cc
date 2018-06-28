// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_handler.hpp"

namespace upgrade {

using namespace std;

HdlrResp UpgHandler::HandleStateUpgReqRcvd(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStateUpgReqRcvd not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStatePreUpgState(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStatePreUpgState not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStatePostBinRestart(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStatePostBinRestart not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStateProcessesQuiesced(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStateProcessesQuiesced not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStateDataplaneDowntimePhase1Start(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStateDataplaneDowntimePhase1Start not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStateDataplaneDowntimePhase2Start(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStateDataplaneDowntimePhase2Start not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStateDataplaneDowntimePhase3Start(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStateDataplaneDowntimePhase3Start not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStateDataplaneDowntimePhase4Start(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStateDataplaneDowntimePhase4Start not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStateCleanup(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStateCleanup not implemented by service");
    return resp;
}

void UpgHandler::HandleStateUpgSuccess(UpgCtx& upgCtx) {
    LogInfo("UpgHandler HandleStateUpgSuccess not implemented by service");
    return;
}

void UpgHandler::HandleStateUpgFailed(UpgCtx& upgCtx) {
    LogInfo("UpgHandler HandleStateUpgFailed not implemented by service");
    return;
}

void UpgHandler::HandleStateUpgAborted(UpgCtx& upgCtx) {
    LogInfo("UpgHandler HandleStateUpgAborted not implemented by service");
    return;
}

} // namespace upgrade
