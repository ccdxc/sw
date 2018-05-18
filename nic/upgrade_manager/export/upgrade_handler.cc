// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_handler.hpp"

namespace upgrade {

using namespace std;

HdlrRespCode UpgHandler::UpgReqStatusCreate(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler UpgReqStatusCreate not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::UpgReqStatusDelete(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler UpgReqStatusDelete not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::HandleStateUpgReqRcvd(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler HandleStateUpgReqRcvd not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::HandleStatePreUpgState(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler HandleStatePreUpgState not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::HandleStatePostBinRestart(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler HandleStatePostBinRestart not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::HandleStateProcessesQuiesced(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler HandleStateProcessesQuiesced not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::HandleStateDataplaneDowntimeStart(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler HandleStateDataplaneDowntimeStart not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::HandleStateCleanup(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler HandleStateCleanup not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::HandleStateUpgSuccess(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler HandleStateUpgSuccess not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::HandleStateUpgFailed(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler HandleStateUpgFailed not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::HandleStateInvalidUpgState(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler HandleStateInvalidUpgState not implemented by service");
    return SUCCESS;
}

} // namespace upgrade
