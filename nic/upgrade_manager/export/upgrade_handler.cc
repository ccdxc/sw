// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_handler.hpp"

namespace upgrade {

using namespace std;

HdlrResp UpgHandler::UpgStateReqCreate(delphi::objects::UpgStateReqPtr req) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler UpgStateReqCreate not implemented by service");
    return resp;
}

HdlrResp UpgHandler::UpgStateReqDelete(delphi::objects::UpgStateReqPtr req) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler UpgStateReqDelete not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStateUpgReqRcvd(delphi::objects::UpgStateReqPtr req) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStateUpgReqRcvd not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStatePreUpgState(delphi::objects::UpgStateReqPtr req) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStatePreUpgState not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStatePostBinRestart(delphi::objects::UpgStateReqPtr req) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStatePostBinRestart not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStateProcessesQuiesced(delphi::objects::UpgStateReqPtr req) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStateProcessesQuiesced not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStateDataplaneDowntimeStart(delphi::objects::UpgStateReqPtr req) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStateDataplaneDowntimeStart not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStateCleanup(delphi::objects::UpgStateReqPtr req) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStateCleanup not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStateUpgSuccess(delphi::objects::UpgStateReqPtr req) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStateUpgSuccess not implemented by service");
    return resp;
}

HdlrResp UpgHandler::HandleStateUpgFailed(delphi::objects::UpgStateReqPtr req) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    LogInfo("UpgHandler HandleStateUpgFailed not implemented by service");
    return resp;
}

} // namespace upgrade
