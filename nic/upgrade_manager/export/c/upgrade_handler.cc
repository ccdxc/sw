// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_handler.hpp"

namespace upgrade {

using namespace std;

HdlrResp UpgHandler::UpgStateReqCreate(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler UpgStateReqCreate not implemented by service");
    return resp;
}

HdlrResp UpgHandler::UpgStateReqDelete(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler UpgStateReqDelete not implemented by service");
    return resp;
}

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

HdlrResp UpgHandler::HandleDataplaneDowntimeAdminQ(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleDataplaneDowntimeAdminQ not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStateDataplaneDowntimePhase2Start(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStateDataplaneDowntimePhase2Start not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStateCleanup(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStateCleanup not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStateUpgSuccess(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStateUpgSuccess not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStateUpgFailed(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStateUpgFailed not implemented by service");
    return resp;
}

} // namespace upgrade
