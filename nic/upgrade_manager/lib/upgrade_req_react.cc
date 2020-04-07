// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade.hpp"
#include "upgrade_req_react.hpp"
#include "upgrade_app_resp_reactor.hpp"
#include "nic/upgrade_manager/include/c/upgrade_state_machine.hpp"
#include "nic/upgrade_manager/include/c/upgrade_metadata.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;
UpgCtx ctx;

UpgStateMachine *StateMachine;

void UpgReqReact::SetStateMachine(delphi::objects::UpgReqPtr req) {
    ctx.upgType = req->upgreqtype();
    ctx.firmwarePkgName = req->upgpkgname();
    ctx.postRestart = false;
    ctx.ctxUpdated = false;
    ctx.upgFailed = false;
    ctx.sysMgr = sysMgr_;
    ctx.haltStateMachine = false;
    if (GetState() >= UpgStatePostRestart) {
        ctx.postRestart = true;
    }
    if (!GetUpgCtxFromMeta(ctx)) {
	UPG_LOG_INFO("GetUpgCtxFromMeta failed");
    }
    if (req->upgreqcmd() == IsUpgPossible) {
        UPG_LOG_INFO("CanUpgrade request restored");
        StateMachine = CanUpgradeStateMachine;
        upgReqType_ = IsUpgPossible;
    } else {
        UPG_LOG_INFO("StartUpgrade request restored");
        if (req->upgreqtype() == UpgTypeDisruptive) {
            StateMachine = DisruptiveUpgradeStateMachine;
        } else {
            StateMachine = NonDisruptiveUpgradeStateMachine;
        }
        upgReqType_ = UpgStart;
    }
}

void UpgReqReact::RegNewApp(string name) {
    CreateUpgradeMetrics();
    if (appRegMap_[name] == false) {
        UPG_LOG_DEBUG("App not registered. Registering {} now.", name);
        appRegMap_[name] = true;
        upgMetric_->NumRegApps()->Set(appRegMap_.size());
    } else {
        UPG_LOG_DEBUG("App {} already registered.", name);
    }
}

void UpgReqReact::UnRegNewApp(string name) {
    if (appRegMap_[name] == true) {
        UPG_LOG_DEBUG("App registered. UnRegistering {} now.", name);
        appRegMap_[name] = false;

        unordered_map<string, bool>::iterator it = appRegMap_.find(name);
        if (it != appRegMap_.end()) {
            appRegMap_.erase(it);
            UPG_LOG_DEBUG("App {} unregistered.", name);
        }
        upgMetric_->NumRegApps()->Set(appRegMap_.size());
    } else {
        UPG_LOG_DEBUG("App {} not registered.", name);
    }
}

UpgReqStateType UpgReqReact::GetState(void) {
    return findUpgStateReq()->upgreqstate();
}

UpgReqStateType UpgReqReact::GetNextState(void) {
    UpgReqStateType  reqType;
    auto reqStatus = findUpgStateReq();
    reqType = reqStatus->upgreqstate();
    if (GetAppRespFail() && (reqType != UpgStateFailed)) {
        UPG_LOG_DEBUG("Some application(s) responded with failure");
        if (upgReqType_ == IsUpgPossible) {
            UPG_LOG_DEBUG("Going to respond back to IsUpgPossible");
            return UpgStateTerminal;
        }
        return UpgStateFailed;
    }
    if (reqType == UpgStateSuccess) 
        upgPassed_ = true;
    return StateMachine[reqType].stateNext;
}

bool UpgReqReact::IsRespTypeFail(UpgStateRespType type) {
    bool ret = false;
    switch (type) {
        case UpgStateCompatCheckRespFail:
        case UpgStateProcessQuiesceRespFail:
        case UpgStatePostRestartRespFail:
        case UpgStateLinkDownRespFail:
        case UpgStateLinkUpRespFail:
        case UpgStateDataplaneDowntimePhase1RespFail:
        case UpgStateDataplaneDowntimePhase2RespFail:
        case UpgStateDataplaneDowntimePhase3RespFail:
        case UpgStateDataplaneDowntimePhase4RespFail:
        case UpgStateSuccessRespFail:
        case UpgStateFailedRespFail:
        case UpgStateAbortRespFail:
        case UpgStateUpgPossibleRespFail:
        case UpgStateHostDownRespFail:
        case UpgStateHostUpRespFail:
        case UpgStatePostHostDownRespFail:
        case UpgStatePostLinkUpRespFail:
        case UpgStateSaveStateRespFail:
            ret = true;
        default:
            break;
    }
    return ret;
}

UpgStateRespType UpgReqReact::GetFailRespType(UpgReqStateType type) {
    return StateMachine[type].stateFailResp;
}

UpgStateRespType UpgReqReact::GetPassRespType(UpgReqStateType type) {
    return StateMachine[type].statePassResp;
}

bool UpgReqReact::CanMoveStateMachine(bool isOnMountReq) {
    UpgStateRespType passType, failType;
    UpgReqStateType  reqType;
    bool ret = true;
    UPG_LOG_DEBUG("Checking if state machine can be moved forward");
    //Find UpgStateReq object
    auto reqStatus = findUpgStateReq();
    reqType = reqStatus->upgreqstate();
    passType = GetPassRespType(reqType);
    failType = GetFailRespType(reqType);
    UPG_LOG_DEBUG("reqType/passType/failType: {}/{}/{}", reqType, passType, failType);

    //check if all responses have come
    vector<delphi::objects::UpgAppRespPtr> upgAppRespList = delphi::objects::UpgAppResp::List(sdk_);
    if (upgAppRespList.size() != appRegMap_.size()) {
        ret = false;
        UPG_LOG_DEBUG("Number of responses from Applications {} is not same as the number of applications {}", upgAppRespList.size(), appRegMap_.size());
    } else {
        for (vector<delphi::objects::UpgAppRespPtr>::iterator appResp=upgAppRespList.begin(); appResp!=upgAppRespList.end(); ++appResp) {
            if (((*appResp)->upgapprespval() != passType) &&
                ((*appResp)->upgapprespval() != failType)){
                UPG_LOG_DEBUG("Application {} still processing {} value {}", (*appResp)->key(), UpgReqStateTypeToStr(reqType), (*appResp)->upgapprespval());
                ret = false;
            } else if ((*appResp)->upgapprespval() == passType) {
                UPG_LOG_DEBUG("Got pass from application {}/{}", (*appResp)->key(), ((*appResp))->meta().ShortDebugString());
            } else {
                UPG_LOG_DEBUG("Got fail from application {} {}", (*appResp)->key(), (*appResp)->upgapprespval());
                UPG_OBFL_TRACE("Got fail from application {} {}", (*appResp)->key(), (*appResp)->upgapprespval());
                ctx.upgFailed = true;
                if (isOnMountReq) {
                    string appRespStr = "App " + (*appResp)->key() + " returned failure: " + (*appResp)->upgapprespstr();
                    AppendAppRespFailStr(appRespStr);
                    SetAppRespFail();
                    UPG_LOG_DEBUG("Adding string {} to list from OnMount", appRespStr);
                }
            }
        }
    }
    if (ret) {
        UPG_LOG_DEBUG("Got pass/fail response from all applications. Can move state machine.");
    }
    return ret;
}

bool UpgReqReact::InvokePreStateHandler(UpgReqStateType reqType) {
    UpgPreStateFunc preStFunc = StateMachine[reqType].preStateFunc;
    if (preStFunc) {
        UPG_LOG_DEBUG("Going to invoke pre-state handler function");
        if (!(preStateHandlers->*preStFunc)(ctx)) {
            UPG_LOG_DEBUG("pre-state handler function returned false");
            return false;
        }
    }
    return true;
}

bool UpgReqReact::InvokePostStateHandler(UpgReqStateType reqType) {
    UpgPostStateFunc postStFunc = StateMachine[reqType].postStateFunc;
    if (postStFunc) {
        UPG_LOG_DEBUG("Going to invoke post-state handler function");
        if (!(postStateHandlers->*postStFunc)(ctx)) {
            UPG_LOG_DEBUG("post-state handler function returned false");
            return false;
        }
    }
    return true;
}

bool UpgReqReact::InvokePrePostStateHandlers(UpgReqStateType reqType) {
    if (!InvokePostStateHandler(reqType)) {
        UPG_LOG_DEBUG("PostState handler returned false");
        return false;
    }
    reqType = GetNextState();
    if (reqType != UpgStateTerminal) {
        if (!InvokePreStateHandler(reqType)) {
            UPG_LOG_DEBUG("PreState handler returned false");
            return false;
        }
    }
    return true;
}

string UpgReqReact::UpgReqStateTypeToStr(UpgReqStateType type) {
    return StateMachine[type].upgReqStateTypeToStr;
}

bool UpgReqReact::GetAppRespFail(void) {
    return appRespFail_;
}

void UpgReqReact::ResetAppResp(void) {
    appRespFail_ = false;
}

void UpgReqReact::SetAppRespFail(void) {
    appRespFail_ = true;
}

void UpgReqReact::AppendAppRespFailStr (string str) {
    appRespFailStrList_.push_back(str);
}

delphi::error UpgReqReact::DeleteUpgMgrResp (void) {
    return upgMgrResp_->DeleteUpgMgrResp();
}
delphi::error UpgReqReact::MoveStateMachine(UpgReqStateType type) {
    if (ctx.haltStateMachine) {
        UPG_LOG_DEBUG("Moving state machine pending switchroot");
        UPG_OBFL_TRACE("Moving state machine pending switchroot");
        return delphi::error::OK();
    }
    //Find UpgStateReq object
    UPG_LOG_DEBUG("UpgReqReact::MoveStateMachine {}", type);
    auto reqStatus = findUpgStateReq();
    reqStatus->set_upgreqstate(type);
    sdk_->SetObject(reqStatus);
    if (type == UpgStateTerminal) {
        UPG_LOG_DEBUG("Upg State Machine reached UpgStateTerminal");
        UpgRespType respType = UpgRespAbort;
        if (upgReqType_ == IsUpgPossible) {
            UPG_LOG_DEBUG("Upg Req of type IsUpgPossible");
            respType = UpgRespUpgPossible;
            if (GetAppRespFail()) {
                upgMetric_->UpgNotPossible()->Incr();
            } else {
                upgMetric_->UpgPossible()->Incr();
            }
        } else {
            UPG_LOG_DEBUG("Upg Req not of type IsUpgPossible");
            if (GetAppRespFail()) {
                respType = UpgRespFail;
                upgMetric_->FailedUpg()->Incr();
            }
            if (upgPassed_ && !upgAborted_) {
                respType = UpgRespPass;
                upgMetric_->SuccessfulUpg()->Incr();
            }
            if (upgAborted_) {
                upgMetric_->AbortedUpg()->Incr();
            }
        }
        upgMgrResp_->UpgradeFinish(respType, appRespFailStrList_);
        if (appRespFailStrList_.empty()) {
            UPG_LOG_DEBUG("Emptied all the responses from applications to agent");
            ResetAppResp();
            upgPassed_ = false;
            upgAborted_ = false;
        }
    }
    if (type != UpgStateTerminal) {
        UPG_LOG_DEBUG("========== Upgrade state moved to {} ==========", UpgReqStateTypeToStr(type));
        UPG_OBFL_TRACE("Upgrade state moved to {}", UpgReqStateTypeToStr(type));
    }
    return delphi::error::OK();
}

void UpgReqReact::DumpFirmwareVersion(void) {
    UPG_OBFL_TRACE("Current firmware meta => buildDate: {}, "\
                                             "buildUser: {}, "\
                                             "baseVersion: {}, "\
                                             "softwareVersion: {]",
                                             ctx.preUpgMeta.buildDate,
                                             ctx.preUpgMeta.buildUser,
                                             ctx.preUpgMeta.baseVersion,
                                             ctx.preUpgMeta.softwareVersion);
    UPG_OBFL_TRACE("Upgrade firmware meta => buildDate: {}, "\
                                             "buildUser: {}, "\
                                             "baseVersion: {}, "\
                                             "softwareVersion: {]",
                                             ctx.postUpgMeta.buildDate,
                                             ctx.postUpgMeta.buildUser,
                                             ctx.postUpgMeta.baseVersion,
                                             ctx.postUpgMeta.softwareVersion);
}

// OnUpgReqCreate gets called when UpgReq object is created
delphi::error UpgReqReact::OnUpgReqCreate(delphi::objects::UpgReqPtr req) {
    UPG_LOG_DEBUG("UpgReq got created for {}/{}", req, req->meta().ShortDebugString());
    CreateUpgradeMetrics();
    ctx.upgType = req->upgreqtype();
    ctx.firmwarePkgName = req->upgpkgname();
    ctx.postRestart = false;
    ctx.ctxUpdated = false;
    ctx.upgFailed = false;
    ctx.sysMgr = sysMgr_;
    ctx.haltStateMachine = false;
    if (!GetUpgCtxFromMeta(ctx)) {
        return delphi::error("GetUpgCtxFromMeta failed");
    }
    UpgReqStateType type = UpgStateCompatCheck;
    DumpFirmwareVersion();
    if (req->upgreqcmd() == IsUpgPossible) {
        UPG_LOG_INFO("CanUpgrade request received");
        UPG_OBFL_TRACE("Check Upgrade Possible");
        StateMachine = CanUpgradeStateMachine;
        upgReqType_ = IsUpgPossible;
        type = UpgStateUpgPossible;
        upgMetric_->IsUpgPossible()->Incr();
    } else {
        UPG_LOG_INFO("StartUpgrade request received");
        if (ctx.upgType == UpgTypeDisruptive) {
            UPG_OBFL_TRACE("Start Disruptive upgrade");
            StateMachine = DisruptiveUpgradeStateMachine;
            upgMetric_->DisruptiveUpg()->Incr();
        } else {
            StateMachine = NonDisruptiveUpgradeStateMachine;
            upgMetric_->NonDisruptiveUpg()->Incr();
        }
        upgReqType_ = UpgStart;
        type = UpgStateCompatCheck;
    }
    if (appRegMap_.size() == 0) {
        AppendAppRespFailStr("No app registered for upgrade");
        upgMgrResp_->UpgradeFinish(UpgRespFail, appRespFailStrList_);
        return delphi::error("No app registered for upgrade");
    }
    // find the status object
    auto upgReqStatus = findUpgStateReq();
    if (upgReqStatus == NULL) {
        // create it since it doesnt exist
        UpgPreStateFunc preStFunc = StateMachine[type].preStateFunc;
        if (preStFunc) {
            UPG_LOG_DEBUG("Going to invoke pre-state handler function");
            if (!(preStateHandlers->*preStFunc)(ctx)) {
                UPG_LOG_DEBUG("pre-state handler function returned false");
                type = UpgStateFailed;
                SetAppRespFail();
                AppendAppRespFailStr("Compat Check Failed. Metadata version mismatch: " + ctx.compatCheckFailureReason);
                upgMgrResp_->UpgradeFinish(UpgRespFail, appRespFailStrList_);
                ResetAppResp();
                upgPassed_ = false;
                upgAborted_ = false;
                return delphi::error::OK();
            }
        }
        RETURN_IF_FAILED(createUpgStateReq(type, req->upgreqtype(), ctx.firmwarePkgName));
    }

    return delphi::error::OK();
}

// OnUpgReqDelete gets called when UpgReq object is deleted
delphi::error UpgReqReact::OnUpgReqDelete(delphi::objects::UpgReqPtr req) {
    UPG_LOG_DEBUG("UpgReq got deleted");
    auto upgReqStatus = findUpgStateReq();
    if (upgReqStatus != NULL) {
        UPG_LOG_DEBUG("Deleting Upgrade Request Status");
        sdk_->DeleteObject(upgReqStatus);
    }
    DeleteUpgMgrResp();
    ctx.ctxUpdated = false;
    return delphi::error::OK();
}

delphi::error UpgReqReact::IsUpgradePossible(delphi::objects::UpgReqPtr req) {
    UPG_LOG_INFO("CanUpgrade request received");
    UPG_OBFL_TRACE("Check Upgrade Possible");

    StateMachine = CanUpgradeStateMachine;
    upgReqType_ = IsUpgPossible;
    UpgReqStateType type = UpgStateUpgPossible;
    upgMetric_->IsUpgPossible()->Incr();
   
    if (appRegMap_.size() == 0) {
        AppendAppRespFailStr("No app registered for upgrade");
        upgMgrResp_->UpgradeFinish(UpgRespFail, appRespFailStrList_);
        return delphi::error("No app registered for upgrade");
    }
    // find the status object
    auto upgReqStatus = findUpgStateReq();
    if (upgReqStatus == NULL) {
        // create it since it doesnt exist
        UpgPreStateFunc preStFunc = StateMachine[type].preStateFunc;
        if (preStFunc) {
            UPG_LOG_DEBUG("Going to invoke pre-state handler function");
            if (!(preStateHandlers->*preStFunc)(ctx)) {
                UPG_LOG_DEBUG("pre-state handler function returned false");
                type = UpgStateFailed;
                SetAppRespFail();
                AppendAppRespFailStr("Compat Check Failed. Metadata version mismatch: " + ctx.compatCheckFailureReason);
                upgMgrResp_->UpgradeFinish(UpgRespFail, appRespFailStrList_);
                ResetAppResp();
                upgPassed_ = false;
                upgAborted_ = false;
                return delphi::error::OK();
            }
        }
        RETURN_IF_FAILED(createUpgStateReq(type, req->upgreqtype(), ctx.firmwarePkgName));
    }

    return delphi::error::OK();
}

delphi::error UpgReqReact::StartUpgrade(delphi::objects::UpgReqPtr req) {
    UpgReqStateType type = UpgStateCompatCheck;
    upgReqType_ = UpgStart;
    delphi::objects::UpgStateReqPtr upgReqStatus = findUpgStateReq();
    if (upgReqStatus != NULL) {
        StateMachine = NonDisruptiveUpgradeStateMachine;
        upgReqStatus->set_upgreqtype(UpgTypeNonDisruptive);
        if (ctx.upgType == UpgTypeDisruptive) {
            StateMachine = DisruptiveUpgradeStateMachine;
            upgReqStatus->set_upgreqtype(UpgTypeDisruptive);
            UPG_OBFL_TRACE("Disruptive upgrade started");
        }
        if (!InvokePreStateHandler(UpgStateCompatCheck)) {
            UPG_LOG_DEBUG("PreState handler returned false");
            return delphi::error("Compat check failed");
        }
        UPG_LOG_DEBUG("Old value {}", upgReqStatus->upgreqstate());
        upgReqStatus->set_upgreqstate(UpgStateCompatCheck);
        upgReqStatus->set_upgpkgname(ctx.firmwarePkgName);
        sdk_->SetObject(upgReqStatus);
        UPG_LOG_DEBUG("Updated Upgrade Request Status UpgStateCompatCheck");
        return delphi::error::OK();
    } else {
        StateMachine = NonDisruptiveUpgradeStateMachine;
        if (ctx.upgType == UpgTypeDisruptive) {
            StateMachine = DisruptiveUpgradeStateMachine;
            UPG_OBFL_TRACE("Disruptive upgrade started");
        }
        // create it since it doesnt exist
        UpgPreStateFunc preStFunc = StateMachine[type].preStateFunc;
        if (preStFunc) {
            UPG_LOG_DEBUG("Going to invoke pre-state handler function");
            if (!(preStateHandlers->*preStFunc)(ctx)) {
                UPG_LOG_DEBUG("pre-state handler function returned false");
                type = UpgStateFailed;
                SetAppRespFail();
                AppendAppRespFailStr("Compat Check Failed. Metadata version mismatch: " + ctx.compatCheckFailureReason);
                upgMgrResp_->UpgradeFinish(UpgRespFail, appRespFailStrList_);
                ResetAppResp();
                upgPassed_ = false;
                upgAborted_ = false;
                return delphi::error::OK();
            }
        }
        RETURN_IF_FAILED(createUpgStateReq(type, req->upgreqtype(), ctx.firmwarePkgName));
    }
    return delphi::error("Did not find UpgStateReqPtr");
}

delphi::error UpgReqReact::AbortUpgrade() {
    delphi::objects::UpgStateReqPtr upgReqStatus = findUpgStateReq();
    if (upgReqStatus != NULL) {
        UPG_OBFL_TRACE("Upgrade aborted");
        upgReqType_ = UpgAbort;
        upgAborted_ = true;
        upgReqStatus->set_upgreqstate(UpgStateAbort);
        upgReqStatus->set_upgpkgname(ctx.firmwarePkgName);
        sdk_->SetObject(upgReqStatus);
        UPG_LOG_DEBUG("Updated Upgrade Request Status UpgAborted");
        return delphi::error::OK();
    }
    return delphi::error("Did not find UpgStateReqPtr");
}

// OnUpgReqCmd gets called when UpgReqCmd attribute changes
delphi::error UpgReqReact::OnUpgReqCmd(delphi::objects::UpgReqPtr req) {
    ctx.upgType = req->upgreqtype();
    ctx.firmwarePkgName = req->upgpkgname();
    ctx.postRestart = false;
    ctx.ctxUpdated = false;
    ctx.upgFailed = false;
    ctx.sysMgr = sysMgr_;
    ctx.haltStateMachine = false;
    // start or abort?
    if (!GetUpgCtxFromMeta(ctx)) {
        return delphi::error("GetUpgCtxFromMeta failed");
    }
    if (req->upgreqcmd() == UpgStart) {
        UPG_OBFL_TRACE("Upgrade modify request for: Start Upgrade");
        UPG_LOG_DEBUG("OnUpgReqCmd got upgType {} firmware {}", ctx.upgType, ctx.firmwarePkgName);
        UPG_LOG_INFO("Start Upgrade");
        return StartUpgrade(req);
    } else if (req->upgreqcmd() == UpgAbort) {
        UPG_OBFL_TRACE("Upgrade modify request for: Abort Upgrade");
        UPG_LOG_INFO("Abort Upgrade");
        return AbortUpgrade();
    } else if (req->upgreqcmd() == IsUpgPossible) {
        UPG_OBFL_TRACE("Upgrade modify request for: IsUpgradePossible");
        UPG_LOG_INFO("Is Upgrade Possible");
        return IsUpgradePossible(req);
    }
    return delphi::error("Cannot decipher the upgreqcmd");
}

// createUpgStateReq creates a upgrade request status object
delphi::error UpgReqReact::createUpgStateReq(UpgReqStateType status, UpgType type, string pkgName) {
    // create an object
    delphi::objects::UpgStateReqPtr req = make_shared<delphi::objects::UpgStateReq>();
    req->set_upgreqstate(status);
    req->set_upgreqtype(type);
    req->set_upgpkgname(pkgName);

    // add it to database
    sdk_->SetObject(req);

    UPG_LOG_DEBUG("Created upgrade request status object for state {} req: {}", status, req);

    return delphi::error::OK();
}

//  ffindUpgReqStat::objects::usinds the upgrade request status object
delphi::objects::UpgStateReqPtr UpgReqReact::findUpgStateReq() {
    return delphi::objects::UpgStateReq::FindObject(sdk_);
}

delphi::objects::UpgReqPtr UpgReqReact::findUpgReq() {
    return delphi::objects::UpgReq::FindObject(sdk_);
}

void UpgReqReact::CreateUpgradeMetrics(void) {
    if (upgMetric_ != NULL) {
        UPG_LOG_DEBUG("UpgradeMetrics object already exists");
        return;
    }
    upgMetric_ = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    assert(upgMetric_ != NULL);
    UPG_LOG_DEBUG("UpgradeMetrics object created");
}
} // namespace upgrade
