// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>
#include <fstream>

#include "upgrade_post_state_handlers.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;

inline bool exists(const std::string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

bool UpgPostStateHandler::PostCompatCheckHandler(UpgCtx &ctx) {
    if (ctx.upgFailed) return true;
    UPG_LOG_DEBUG("UpgPostStateHandler PostCompatCheck returning");
    return true;
}

bool UpgPostStateHandler::PostPostRestartHandler(UpgCtx &ctx) {
    if (ctx.upgFailed) return true;
    UPG_OBFL_TRACE("Upgrade state machine running after switchroot");
    UPG_LOG_DEBUG("UpgPostStateHandler PostPostRestart returning");
    return true;
}

bool UpgPostStateHandler::PostProcessQuiesceHandler(UpgCtx &ctx) {
    if (ctx.upgFailed) return true;
    UPG_LOG_DEBUG("UpgPostStateHandler PostProcessesQuiesced returning");
    return true;
}

bool UpgPostStateHandler::PostLinkDownHandler(UpgCtx &ctx) {
    if (ctx.upgFailed) return true;
    UPG_LOG_DEBUG("UpgPostStateHandler Link Down returning");
    return true;
}

bool UpgPostStateHandler::PostLinkUpHandler(UpgCtx &ctx) {
    if (ctx.upgFailed) return true;
    UPG_LOG_DEBUG("UpgPostStateHandler PostLinkUpHandler returning");
    return true;
}

bool UpgPostStateHandler::PostDataplaneDowntimePhase1Handler(UpgCtx &ctx) {
    if (ctx.upgFailed) return true;
    UPG_LOG_DEBUG("UpgPostStateHandler PostDataplaneDowntimePhase1 returning");
    return true;
}

bool UpgPostStateHandler::PostDataplaneDowntimePhase2Handler(UpgCtx &ctx) {
    if (ctx.upgFailed) return true;
    UPG_LOG_DEBUG("UpgPostStateHandler PostDataplaneDowntimePhase2 returning");
    return true;
}

bool UpgPostStateHandler::PostDataplaneDowntimePhase3Handler(UpgCtx &ctx) {
    if (ctx.upgFailed) return true;
    UPG_LOG_DEBUG("UpgPostStateHandler PostDataplaneDowntimePhase3 returning");
    return true;
}

bool UpgPostStateHandler::PostDataplaneDowntimePhase4Handler(UpgCtx &ctx) {
    if (ctx.upgFailed) return true;
    UPG_LOG_DEBUG("UpgPostStateHandler PostDataplaneDowntimePhase4 returning");
    return true;
}

bool UpgPostStateHandler::PostSuccessHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPostStateHandler PostSuccess returning");
    UPG_OBFL_TRACE("Upgrade successful");
    if (exists("/nic/tools/fwupdate") && exists("/update/upgrade_halt_state_machine")) {
        remove("/update/upgrade_halt_state_machine");
    }
    return true;
}

bool UpgPostStateHandler::PostFailedHandler(UpgCtx &ctx) {
    UPG_OBFL_TRACE("Upgrade failed");
    UPG_LOG_DEBUG("UpgPostStateHandler PostFailed returning");
    if (exists("/nic/tools/fwupdate") && exists("/update/upgrade_halt_state_machine")) {
        remove("/update/upgrade_halt_state_machine");
    }
    return true;
}

bool UpgPostStateHandler::PostAbortHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPostStateHandler PostAborted returning");
    return true;
}

bool UpgPostStateHandler::PostHostDowntHandler(UpgCtx &ctx) {
    if (ctx.upgFailed) return true;
    UPG_LOG_DEBUG("UpgPostStateHandler PostHostDownt returning");
    return true;
}

bool UpgPostStateHandler::PostHostUpHandler(UpgCtx &ctx) {
    if (ctx.upgFailed) return true;
    UPG_LOG_DEBUG("UpgPostStateHandler PostHostUp returning");
    return true;
}

bool UpgPostStateHandler::PostPostHostDownHandler(UpgCtx &ctx) {
    if (ctx.upgFailed) return true;
    UPG_LOG_DEBUG("UpgPostStateHandler PostPostHostDown returning");
    return true;
}

bool UpgPostStateHandler::PostPostLinkUpHandler(UpgCtx &ctx) {
    if (ctx.upgFailed) return true;
    UPG_LOG_DEBUG("UpgPostStateHandler PostPostLinkUp returning");
    return true;
}

bool UpgPostStateHandler::PostSaveStateHandler(UpgCtx &ctx) {
    UPG_LOG_DEBUG("UpgPostStateHandler PostSaveState returning");
    if (ctx.upgFailed) { 
        UPG_OBFL_TRACE("Going to restart apps in a bit");
        return true;
    }
    UPG_OBFL_TRACE("Going to restart system");
    if (exists("/nic/tools/fwupdate") && !exists("/update/upgrade_halt_state_machine")) {
        int i = 0;
        do {
            UPG_LOG_DEBUG("Trying to create /update/upgrade_halt_state_machine file");
            ofstream file;
            file.open("/update/upgrade_halt_state_machine");
            file << "going to halt state machine for switchroot\n";
            file.close();
            int ret = 0;
            string cmd = "sync";
            if ((ret = system (cmd.c_str())) != 0) {
                UPG_LOG_INFO("Unable to sync");
            }
            sleep(1);
            i++;
        } while ((i<=10) && (!exists("/update/upgrade_halt_state_machine")));
        if (exists("/update/upgrade_halt_state_machine")) {
            UPG_OBFL_TRACE("File /update/upgrade_halt_state_machine got created. Switching root now.");  
        } else {
            UPG_OBFL_TRACE("File /update/upgrade_halt_state_machine did not get created.");  
            return false;
        }
        UPG_LOG_DEBUG("File created");
        ctx.haltStateMachine = true;
        ctx.sysMgr->restart_system();
    }
    return true;
}
} // namespace upgrade
