// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_handler.hpp"

namespace upgrade {

using namespace std;

HdlrRespCode UpgHandler::UpgStateReqCreate(delphi::objects::UpgStateReqPtr req) {
    LogInfo("UpgHandler UpgStateReqCreate not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::UpgStateReqDelete(delphi::objects::UpgStateReqPtr req) {
    LogInfo("UpgHandler UpgStateReqDelete not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::HandleStateUpgReqRcvd(delphi::objects::UpgStateReqPtr req) {
    LogInfo("UpgHandler HandleStateUpgReqRcvd not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::HandleStatePreUpgState(delphi::objects::UpgStateReqPtr req) {
    LogInfo("UpgHandler HandleStatePreUpgState not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::HandleStatePostBinRestart(delphi::objects::UpgStateReqPtr req) {
    LogInfo("UpgHandler HandleStatePostBinRestart not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::HandleStateProcessesQuiesced(delphi::objects::UpgStateReqPtr req) {
    LogInfo("UpgHandler HandleStateProcessesQuiesced not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::HandleStateDataplaneDowntimeStart(delphi::objects::UpgStateReqPtr req) {
    LogInfo("UpgHandler HandleStateDataplaneDowntimeStart not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::HandleStateCleanup(delphi::objects::UpgStateReqPtr req) {
    LogInfo("UpgHandler HandleStateCleanup not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::HandleStateUpgSuccess(delphi::objects::UpgStateReqPtr req) {
    LogInfo("UpgHandler HandleStateUpgSuccess not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::HandleStateUpgFailed(delphi::objects::UpgStateReqPtr req) {
    LogInfo("UpgHandler HandleStateUpgFailed not implemented by service");
    return SUCCESS;
}

HdlrRespCode UpgHandler::HandleStateInvalidUpgState(delphi::objects::UpgStateReqPtr req) {
//    LogInfo("UpgHandler HandleStateInvalidUpgState not implemented by service");
    return SUCCESS;
}

} // namespace upgrade
