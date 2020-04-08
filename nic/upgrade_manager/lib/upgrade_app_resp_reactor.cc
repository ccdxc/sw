// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade.hpp"
#include "upgrade_req_react.hpp"
#include "upgrade_app_resp_reactor.hpp"
#include "nic/upgrade_manager/include/c/upgrade_state_machine.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;

inline bool exists(const std::string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

// OnUpgAppRespCreate gets called when UpgAppResp object is created
delphi::error UpgAppRespReact::OnUpgAppRespCreate(delphi::objects::UpgAppRespPtr resp) {
    UPG_LOG_DEBUG("UpgAppResp got created for {}/{}", resp, resp->meta().ShortDebugString());
    if (upgMgr_->IsRespTypeFail(resp->upgapprespval())) {
        string appRespStr = "App " + resp->key() + " returned failure: " + resp->upgapprespstr();
        upgMgr_->AppendAppRespFailStr(appRespStr);
        upgMgr_->SetAppRespFail();
        UPG_LOG_DEBUG("Adding string {} to list", appRespStr);
    }
    if (upgMgr_->CanMoveStateMachine(false)) {
        UPG_LOG_DEBUG("Can move state machine. Invoking post-state handler.");
        auto upgStateReq = upgMgr_->findUpgStateReq();
        UpgReqStateType type = upgStateReq->upgreqstate();
        if (!upgMgr_->InvokePrePostStateHandlers(type)) {
            UPG_LOG_DEBUG("PrePostState handlers returned false");
            type = UpgStateFailed;
            upgMgr_->SetAppRespFail();
        } else {
            type = upgMgr_->GetNextState();
        }
        return upgMgr_->MoveStateMachine(type);
    }
    UPG_LOG_DEBUG("Cannot move state machine yet");
    return delphi::error::OK();
}

extern UpgCtx ctx;

delphi::error UpgAppRespReact::OnUpgAppRespDelete(delphi::objects::UpgAppRespPtr resp) {
    UPG_LOG_DEBUG("UpgAppResp got deleted for {}/{}", resp, resp->meta().ShortDebugString());
    vector<delphi::objects::UpgAppRespPtr> upgAppResplist = delphi::objects::UpgAppResp::List(sdk_);
    int ret = 0;
    string cmd;
    if (upgAppResplist.empty()) {
        UPG_LOG_DEBUG("All UpgAppResp objects got deleted");
        upgMgr_->DeleteUpgMgrResp();
        if (exists("/nic/tools/fwupdate")) {
            UPG_LOG_INFO("Image is: {}", ctx.firmwarePkgName);
        }
        if (ctx.upgFailed) {
            //TODO: Move to OnUpgRespDelete once we have go APIs from sysmgr
            if (ctx.postRestart) {
                UPG_LOG_DEBUG("Upgrade failed last time. So going to switch back to old code.");
                UPG_OBFL_TRACE("Upgrade failed last time. So going to switch back to old code.");
                sleep(5);
                if (exists("/nic/tools/fwupdate")) {
                    ctx.sysMgr->restart_system();
                } 
            } else if (ctx.upgPostCompatCheck) {
                UPG_LOG_DEBUG("Upgrade failed last time. So going to restart apps.");
                UPG_OBFL_TRACE("Upgrade failed last time. So going to restart apps.");
                sleep(5);
                if (exists("/nic/tools/fwupdate")) {
                    cmd = "rm -rf /tmp/naples-netagent.db";
                    if ((ret = system (cmd.c_str())) != 0) {
                        UPG_LOG_INFO("Unable to delete naples-netagent.db");
                    }
                    cmd = "/nic/tools/rollback_cleanup.sh";
                    if ((ret = system (cmd.c_str())) != 0) {
                        UPG_LOG_INFO("Unable to reload mnic driver");
                    }
                    UPG_LOG_DEBUG("Calling respawn_processes");
                    UPG_OBFL_TRACE("Calling respawn_processes");
                    ctx.sysMgr->respawn_processes();
                }
            } else {
                UPG_LOG_DEBUG("UpgFailed during compat check.");
                UPG_OBFL_TRACE("UpgFailed during compat check.");
            }
        }
    }
    UPG_LOG_DEBUG("Returning from OnUpgAppRespDelete");

    return delphi::error::OK();
}


string UpgAppRespReact::UpgStateRespTypeToStr(UpgStateRespType type) {
    return GetAppRespStrUtil(type, ctx.upgType);
}

delphi::error UpgAppRespReact::OnUpgAppRespVal(delphi::objects::UpgAppRespPtr resp) {
    if (UpgStateRespTypeToStr(resp->upgapprespval()) != "") {
        UPG_LOG_DEBUG("\n\n\n========== Got Response {} from {} application ==========", UpgStateRespTypeToStr(resp->upgapprespval()), resp->key());
        UPG_OBFL_TRACE("Got Response {} from {} application", UpgStateRespTypeToStr(resp->upgapprespval()), resp->key());
    }
    //UPG_LOG_DEBUG("UpgAppRespReact OnUpgAppRespVal got called for {}/{}/{}", 
                         //resp, resp->meta().ShortDebugString(), resp->upgapprespval());

    if (upgMgr_->IsRespTypeFail(resp->upgapprespval())) {
        string appRespStr = "App " + resp->key() + " returned failure: " + resp->upgapprespstr();
        upgMgr_->AppendAppRespFailStr(appRespStr);
        upgMgr_->SetAppRespFail();
        UPG_LOG_DEBUG("Adding string {} to list", appRespStr);
    }

    if (upgMgr_->CanMoveStateMachine(false)) {
        UPG_LOG_DEBUG("Can move state machine. Invoking post-state handler.");
        auto upgStateReq = upgMgr_->findUpgStateReq();
        UpgReqStateType type = upgStateReq->upgreqstate();
        if (!upgMgr_->InvokePrePostStateHandlers(type)) {
            UPG_LOG_DEBUG("PrePostState handlers returned false");
            type = UpgStateFailed;
            upgMgr_->SetAppRespFail();
        } else {
            type = upgMgr_->GetNextState();
        }
        return upgMgr_->MoveStateMachine(type);
    } else {
        UPG_LOG_DEBUG("Cannot move state machine yet");
        return delphi::error::OK();
    }
}

} // namespace upgrade
