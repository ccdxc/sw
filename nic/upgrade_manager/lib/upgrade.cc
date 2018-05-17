// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade.hpp"

namespace upgrade {

using namespace std;

// OnUpgAppRespCreate gets called when UpgAppResp object is created
delphi::error UpgAppRespHdlr::OnUpgAppRespCreate(delphi::objects::UpgAppRespPtr resp) {
    LogInfo("UpgAppResp got created for {}/{}", resp, resp->meta().ShortDebugString());

    return delphi::error::OK();
}

UpgReqStateType UpgradeMgr::GetNextState(void) {
    UpgReqStateType  reqType, nextReqType;
    vector<delphi::objects::UpgReqStatusPtr> upgReqStatusList = delphi::objects::UpgReqStatus::List(sdk_);
    for (vector<delphi::objects::UpgReqStatusPtr>::iterator reqStatus=upgReqStatusList.begin(); reqStatus!=upgReqStatusList.end(); ++reqStatus) {
        reqType = (*reqStatus)->upgreqstate();
        break;
    }
    switch (reqType) {
        case InvalidUpgState:
            nextReqType = InvalidUpgState;
            break;
        case UpgReqRcvd:
            nextReqType = PreUpgState;
            break;
        case PreUpgState:
            nextReqType = ProcessesQuiesced;
            break;
        case ProcessesQuiesced:
            nextReqType = PostBinRestart;
            break;
        case PostBinRestart:
            nextReqType = DataplaneDowntimeStart;
            break;
        case DataplaneDowntimeStart:
            //TODO set UpgSuccess/UpgFailed
            nextReqType = Cleanup;
            break;
        case Cleanup:
            nextReqType = InvalidUpgState;
            break;
        case UpgSuccess:
            nextReqType = Cleanup;
            break;
        case UpgFailed:
            nextReqType = Cleanup;
            break;
        default:
            nextReqType = InvalidUpgState;
            break;
    }
    return nextReqType;
}

UpgRespStateType UpgradeMgr::GetFailRespType(UpgReqStateType type) {
    UpgRespStateType ret;
    switch(type) {
        case InvalidUpgState:
            ret = InvalidUpgStateFail;
            break;
        case UpgReqRcvd:
            ret = UpgReqRcvdFail;
            break;
        case PreUpgState:
            ret = PreUpgStateFail;
            break;
        case ProcessesQuiesced:
            ret = ProcessesQuiescedFail;
            break;
        case PostBinRestart:
            ret = PostBinRestartFail;
            break;
        case DataplaneDowntimeStart:
            ret = DataplaneDowntimeStartFail;
            break;
        case Cleanup:
            ret = CleanupFail;
            break;
        case UpgSuccess:
            ret = UpgSuccessFail;
            break;
        case UpgFailed:
            ret = UpgFailedFail;
            break;
        default:
            ret = InvalidUpgStateFail;
            break;
    }
    return ret;
}

UpgRespStateType UpgradeMgr::GetPassRespType(UpgReqStateType type) {
    UpgRespStateType ret;
    switch(type) {
        case InvalidUpgState:
            ret = InvalidUpgStatePass;
            break;
        case UpgReqRcvd:
            ret = UpgReqRcvdPass;
            break;
        case PreUpgState:
            ret = PreUpgStatePass;
            break;
        case ProcessesQuiesced:
            ret = ProcessesQuiescedPass;
            break;
        case PostBinRestart:
            ret = PostBinRestartPass;
            break;
        case DataplaneDowntimeStart:
            ret = DataplaneDowntimeStartPass;
            break;
        case Cleanup:
            ret = CleanupPass;
            break;
        case UpgSuccess:
            ret = UpgSuccessPass;
            break;
        case UpgFailed:
            ret = UpgFailedPass;
            break;
        default:
            ret = InvalidUpgStatePass;
            break;
    }
    return ret;
}

bool UpgradeMgr::CanMoveStateMachine(void) {
    UpgRespStateType passType, failType;
    UpgReqStateType  reqType;
    bool ret = true;
    LogInfo("UpgradeMgr::CanMoveStateMachine called");
    //Find UpgReqStatus object
    vector<delphi::objects::UpgReqStatusPtr> upgReqStatusList = delphi::objects::UpgReqStatus::List(sdk_);
    for (vector<delphi::objects::UpgReqStatusPtr>::iterator reqStatus=upgReqStatusList.begin(); reqStatus!=upgReqStatusList.end(); ++reqStatus) {
        reqType = (*reqStatus)->upgreqstate();
        passType = GetPassRespType(reqType);
        failType = GetFailRespType(reqType);
        LogInfo("reqType/passType/failType: {}/{}/{}", reqType, passType, failType);
    }

    //check if all responses have come
    vector<delphi::objects::UpgAppRespPtr> upgAppRespList = delphi::objects::UpgAppResp::List(sdk_);
    for (vector<delphi::objects::UpgAppRespPtr>::iterator appResp=upgAppRespList.begin(); appResp!=upgAppRespList.end(); ++appResp) {
        LogInfo("Got application {}", (*appResp)->key());
        if (((*appResp)->upgapprespval() != passType) &&
            ((*appResp)->upgapprespval() != failType)){
            LogInfo("Application not done processing {}", reqType);
            ret = false;
        }
    }
    return ret;
}

delphi::error UpgradeMgr::MoveStateMachine(UpgReqStateType type) {
    //Find UpgReqStatus object
    LogInfo("UpgradeMgr::MoveStateMachine");
    vector<delphi::objects::UpgReqStatusPtr> upgReqStatusList = delphi::objects::UpgReqStatus::List(sdk_);
    for (vector<delphi::objects::UpgReqStatusPtr>::iterator reqStatus=upgReqStatusList.begin(); reqStatus!=upgReqStatusList.end(); ++reqStatus) {
        LogInfo("Setting next UpgReqStateType to {}", type);
        (*reqStatus)->set_upgreqstate(type);
        sdk_->SetObject(*reqStatus);
    }
    return delphi::error::OK();
}

delphi::error UpgAppRespHdlr::OnUpgAppRespVal(delphi::objects::UpgAppRespPtr resp) {
    LogInfo("UpgAppRespHdlr OnUpgAppRespVal got called for {}/{}/{}", 
                         resp, resp->meta().ShortDebugString(), resp->upgapprespval());

    if (this->upgMgr_->CanMoveStateMachine()) {
        return (this->upgMgr_->MoveStateMachine(this->upgMgr_->GetNextState()));
    } else {
        LogInfo("Cannot move state machine yet");
        return delphi::error::OK();
    }
}

// OnUpgReqCreate gets called when UpgReq object is created
delphi::error UpgradeMgr::OnUpgReqCreate(delphi::objects::UpgReqPtr req) {
    LogInfo("UpgReq got created for {}/{}", req, req->meta().ShortDebugString());

    // find the status object
    auto upgReqStatus = this->findUpgReqStatus(req->key().id());
    if (upgReqStatus == NULL) {
        // create it since it doesnt exist
        RETURN_IF_FAILED(this->createUpgReqStatus(req->key().id(), upgrade::InvalidUpgState));
    }

    return delphi::error::OK();
}

// OnUpgReqDelete gets called when UpgReq object is deleted
delphi::error UpgradeMgr::OnUpgReqDelete(delphi::objects::UpgReqPtr req) {
    LogInfo("UpgReq got deleted");
    auto upgReqStatus = this->findUpgReqStatus(req->key().id());
    if (upgReqStatus != NULL) {
        LogInfo("Deleting Upgrade Request Status");
        sdk_->DeleteObject(upgReqStatus);
    }
    return delphi::error::OK();
}

// OnUpgReqCmd gets called when UpgReqCmd attribute changes
delphi::error UpgradeMgr::OnUpgReqCmd(delphi::objects::UpgReqPtr req) {
    // start or abort?
    if (req->upgreqcmd() == upgrade::UpgStart) {
        LogInfo("Start Upgrade");
    } else {
        LogInfo("Abort Upgrade");
    }

    // set the oper state on status object
    delphi::objects::UpgReqStatusPtr upgReqStatus = this->findUpgReqStatus(req->key().id());
    if (upgReqStatus != NULL) {
        upgReqStatus->set_upgreqstate(upgrade::UpgReqRcvd);
        sdk_->SetObject(upgReqStatus);
        LogInfo("Updated Upgrade Request Status UpgReqRcvd");
    }

    return delphi::error::OK();
}

// createUpgReqStatus creates a upgrade request status object
delphi::error UpgradeMgr::createUpgReqStatus(uint32_t id, upgrade::UpgReqStateType status) {
    // create an object
    delphi::objects::UpgReqStatusPtr req = make_shared<delphi::objects::UpgReqStatus>();
    req->set_key(id);
    req->set_upgreqstate(status);

    // add it to database
    sdk_->SetObject(req);

    LogInfo("Created upgrade request status object for id {} state {} req: {}", id, status, req);

    return delphi::error::OK();
}

//  ffindUpgReqStat::objects::usinds the upgrade request status object
delphi::objects::UpgReqStatusPtr UpgradeMgr::findUpgReqStatus(uint32_t id) {
    delphi::objects::UpgReqStatusPtr req = make_shared<delphi::objects::UpgReqStatus>();
    req->set_key(id);

    // find the object
    delphi::BaseObjectPtr obj = sdk_->FindObject(req);

    return static_pointer_cast<delphi::objects::UpgReqStatus>(obj);
}

// UpgradeService constructor
UpgradeService::UpgradeService(delphi::SdkPtr sk) : UpgradeService(sk, "UpgradeService") {
}

// UpgradeService constructor
UpgradeService::UpgradeService(delphi::SdkPtr sk, string name) {
    // save a pointer to sdk
    this->sdk_ = sk;
    this->svcName_ = name;

    // mount objects
    delphi::objects::UpgReq::Mount(sdk_, delphi::ReadWriteMode);
    delphi::objects::UpgReqStatus::Mount(sdk_, delphi::ReadWriteMode);
    delphi::objects::UpgAppResp::Mount(sdk_, delphi::ReadMode);

    // create upgrade manager event handler
    upgMgr_ = make_shared<UpgradeMgr>(sdk_);

    upgAppRespHdlr_ = make_shared<UpgAppRespHdlr>(sdk_, upgMgr_);

    // Register upgrade request reactor
    delphi::objects::UpgReq::Watch(sdk_, upgMgr_);
    delphi::objects::UpgAppResp::Watch(sdk_, upgAppRespHdlr_);

    LogInfo("Upgrade service constructor got called");
}

// OnMountComplete gets called when all the objects are mounted
void UpgradeService::OnMountComplete() {
    LogInfo("UpgradeService OnMountComplete got called\n");

    // walk all upgrade request objects and reconcile them
    vector<delphi::objects::UpgReqPtr> upgReqlist = delphi::objects::UpgReq::List(sdk_);
    for (vector<delphi::objects::UpgReqPtr>::iterator req=upgReqlist.begin(); req!=upgReqlist.end(); ++req) {
        upgMgr_->OnUpgReqCreate(*req);
    }

    LogInfo("============== UpgradeService Finished Reconciliation ==================\n");
}

// createUpgReqSpec creates a dummy upgrade request 
void UpgradeService::createUpgReqSpec() {
    // create an object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    srand(time(NULL));
    req->mutable_key()->set_id(rand());
    req->set_upgreqcmd(upgrade::InvalidCmd);

    // add it to database
    sdk_->SetObject(req);

    req->set_upgreqcmd(upgrade::UpgStart);
    sdk_->SetObject(req);
}

// createTimerHandler creates a dummy code upgrade request
void UpgradeService::createTimerHandler(ev::timer &watcher, int revents) {
    LogInfo("Creating a dummy upgrade request");

    // create a dummy upgrade request
    this->createUpgReqSpec();
}
} // namespace upgrade
