// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade.hpp"
#include "upgrade_mgr.hpp"
#include "upgrade_app_resp_handlers.hpp"

namespace upgrade {

using namespace std;

UpgReqStateType UpgradeMgr::GetNextState(void) {
    UpgReqStateType  reqType, nextReqType;
    vector<delphi::objects::UpgStateReqPtr> upgReqStatusList = delphi::objects::UpgStateReq::List(sdk_);
    for (vector<delphi::objects::UpgStateReqPtr>::iterator reqStatus=upgReqStatusList.begin(); reqStatus!=upgReqStatusList.end(); ++reqStatus) {
        reqType = (*reqStatus)->upgreqstate();
        break;
    }
    switch (reqType) {
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
            nextReqType = UpgSuccess;
            break;
        case Cleanup:
            nextReqType = UpgStateTerminal;
            break;
        case UpgSuccess:
            nextReqType = Cleanup;
            break;
        case UpgFailed:
            nextReqType = Cleanup;
            break;
        default:
            nextReqType = UpgStateTerminal;
            break;
    }
    return nextReqType;
}

UpgRespStateType UpgradeMgr::GetFailRespType(UpgReqStateType type) {
    UpgRespStateType ret;
    switch(type) {
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
            ret = UpgSuccessFail;
            break;
    }
    return ret;
}

UpgRespStateType UpgradeMgr::GetPassRespType(UpgReqStateType type) {
    UpgRespStateType ret;
    switch(type) {
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
            ret = UpgSuccessPass;
            break;
    }
    return ret;
}

bool UpgradeMgr::CanMoveStateMachine(void) {
    UpgRespStateType passType, failType;
    UpgReqStateType  reqType;
    bool ret = true;
    LogInfo("Checking if state machine can be moved forward");
    //Find UpgStateReq object
    vector<delphi::objects::UpgStateReqPtr> upgReqStatusList = delphi::objects::UpgStateReq::List(sdk_);
    for (vector<delphi::objects::UpgStateReqPtr>::iterator reqStatus=upgReqStatusList.begin(); reqStatus!=upgReqStatusList.end(); ++reqStatus) {
        reqType = (*reqStatus)->upgreqstate();
        passType = GetPassRespType(reqType);
        failType = GetFailRespType(reqType);
        //LogInfo("reqType/passType/failType: {}/{}/{}", reqType, passType, failType);
    }

    //check if all responses have come
    vector<delphi::objects::UpgAppRespPtr> upgAppRespList = delphi::objects::UpgAppResp::List(sdk_);
    for (vector<delphi::objects::UpgAppRespPtr>::iterator appResp=upgAppRespList.begin(); appResp!=upgAppRespList.end(); ++appResp) {
        if (((*appResp)->upgapprespval() != passType) &&
            ((*appResp)->upgapprespval() != failType)){
            LogInfo("Application {} still processing {}", (*appResp)->key(), UpgReqStateTypeToStr(reqType));
            ret = false;
        } else if ((*appResp)->upgapprespval() == passType) {
            LogInfo("Got pass from application {}", (*appResp)->key());
        } else {
            LogInfo("Got fail from application {}", (*appResp)->key());
        }
    }
    if (ret) {
        LogInfo("Got pass response from all applications. Can move state machine.");
    }
    return ret;
}

string UpgradeMgr::UpgReqStateTypeToStr(UpgReqStateType type) {
    switch (type) {
        case UpgReqRcvd:
            return "Upgrade Request Received";
        case PreUpgState:
            return "Perform Compat Check";
        case ProcessesQuiesced:
            return "Quiesce Processes Pre-Restart";
        case PostBinRestart:
            return "Post Process Restart";
        case DataplaneDowntimeStart:
            return "Dataplane Downtime Start";
        case Cleanup:
            return "Cleanup State";
        case UpgSuccess:
            return "Upgrade Success";
        case UpgFailed:
            return "Upgrade Fail";
        default:
            return "";
    }
}

delphi::error UpgradeMgr::DeleteUpgMgrResp (void) {
    return this->upgMgrResp_->DeleteUpgMgrResp();
}
delphi::error UpgradeMgr::MoveStateMachine(UpgReqStateType type) {
    //Find UpgStateReq object
    LogInfo("UpgradeMgr::MoveStateMachine");
    vector<delphi::objects::UpgStateReqPtr> upgReqStatusList = delphi::objects::UpgStateReq::List(sdk_);
    for (vector<delphi::objects::UpgStateReqPtr>::iterator reqStatus=upgReqStatusList.begin(); reqStatus!=upgReqStatusList.end(); ++reqStatus) {
        (*reqStatus)->set_upgreqstate(type);
        sdk_->SetObject(*reqStatus);
    }
    if ((type == UpgSuccess) || (type == UpgFailed)) {
        //Notify Agent
        this->upgMgrResp_->UpgradeFinish(type == UpgSuccess);
    }
    if (type != UpgStateTerminal)
        LogInfo("========== Upgrade state moved to {} ==========", UpgReqStateTypeToStr(type));
    return delphi::error::OK();
}

// OnUpgReqCreate gets called when UpgReq object is created
delphi::error UpgradeMgr::OnUpgReqCreate(delphi::objects::UpgReqPtr req) {
    LogInfo("UpgReq got created for {}/{}", req, req->meta().ShortDebugString());

    // find the status object
    auto upgReqStatus = this->findUpgStateReq(req->key());
    if (upgReqStatus == NULL) {
        // create it since it doesnt exist
        RETURN_IF_FAILED(this->createUpgStateReq(req->key(), upgrade::UpgReqRcvd));
    }

    return delphi::error::OK();
}

// OnUpgReqDelete gets called when UpgReq object is deleted
delphi::error UpgradeMgr::OnUpgReqDelete(delphi::objects::UpgReqPtr req) {
    LogInfo("UpgReq got deleted");
    auto upgReqStatus = this->findUpgStateReq(req->key());
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
    delphi::objects::UpgStateReqPtr upgReqStatus = this->findUpgStateReq(req->key());
    if (upgReqStatus != NULL) {
        upgReqStatus->set_upgreqstate(upgrade::UpgReqRcvd);
        sdk_->SetObject(upgReqStatus);
        LogInfo("Updated Upgrade Request Status UpgReqRcvd");
    }

    return delphi::error::OK();
}

// createUpgStateReq creates a upgrade request status object
delphi::error UpgradeMgr::createUpgStateReq(uint32_t id, upgrade::UpgReqStateType status) {
    // create an object
    delphi::objects::UpgStateReqPtr req = make_shared<delphi::objects::UpgStateReq>();
    req->set_key(id);
    req->set_upgreqstate(status);

    // add it to database
    sdk_->SetObject(req);

    LogInfo("Created upgrade request status object for id {} state {} req: {}", id, status, req);

    return delphi::error::OK();
}

//  ffindUpgReqStat::objects::usinds the upgrade request status object
delphi::objects::UpgStateReqPtr UpgradeMgr::findUpgStateReq(uint32_t id) {
    delphi::objects::UpgStateReqPtr req = make_shared<delphi::objects::UpgStateReq>();
    req->set_key(id);

    // find the object
    delphi::BaseObjectPtr obj = sdk_->FindObject(req);

    return static_pointer_cast<delphi::objects::UpgStateReq>(obj);
}

} // namespace upgrade
