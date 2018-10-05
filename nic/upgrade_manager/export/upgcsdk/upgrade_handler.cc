// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_handler.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;

HdlrResp UpgHandler::CompatCheckHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler CompatCheck not implemented by service");
    return resp;
}

HdlrResp UpgHandler::PostRestartHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler PostRestart not implemented by service");
    return resp;
}

HdlrResp UpgHandler::ProcessQuiesceHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler ProcessesQuiesced not implemented by service");
    return resp;
}

HdlrResp UpgHandler::LinkDownHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler ProcessesQuiesced not implemented by service");
    return resp;
}

HdlrResp UpgHandler::DataplaneDowntimePhase1Handler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler DataplaneDowntimePhase1 not implemented by service");
    return resp;
}

HdlrResp UpgHandler::DataplaneDowntimePhase2Handler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler DataplaneDowntimePhase2 not implemented by service");
    return resp;
}

HdlrResp UpgHandler::DataplaneDowntimePhase3Handler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler DataplaneDowntimePhase3 not implemented by service");
    return resp;
}

HdlrResp UpgHandler::DataplaneDowntimePhase4Handler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler DataplaneDowntimePhase4 not implemented by service");
    return resp;
}

HdlrResp UpgHandler::CleanupHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler Cleanup not implemented by service");
    return resp;
}

void UpgHandler::SuccessHandler(UpgCtx& upgCtx) {
    UPG_LOG_DEBUG("UpgHandler Success not implemented by service");
    return;
}

void UpgHandler::FailedHandler(UpgCtx& upgCtx) {
    UPG_LOG_DEBUG("UpgHandler Failed not implemented by service");
    return;
}

void UpgHandler::AbortHandler(UpgCtx& upgCtx) {
    UPG_LOG_DEBUG("UpgHandler Abort not implemented by service");
    return;
}

HdlrResp UpgHandler::LinkUpHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler LinkUp not implemented by service");
    return resp;
}

HdlrResp UpgHandler::DataplaneDowntimeStartHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler DataplaneDowntimeStart not implemented by service");
    return resp;
}

HdlrResp UpgHandler::IsSystemReadyHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    UPG_LOG_DEBUG("UpgHandler IsSystemReady not implemented by service");
    return resp;
}

} // namespace upgrade
