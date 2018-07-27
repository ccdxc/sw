// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;
using namespace delphi::objects;

void UpgSdk::SendAppRespSuccess(void) {
    UPG_LOG_DEBUG("Application returning success via UpgSdk");
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    auto upgStateReqPtr = upgAppRespPtr_->GetUpgStateReqPtr();
    if (upgStateReqPtr == NULL) {
        UPG_LOG_ERROR("upgStateReqPtr is NULL");
        return;
    }
    UpgReqStateType reqType = upgStateReqPtr->upgreqstate(); 
    UpgStateRespType respType = upgAppRespPtr_->GetUpgAppRespNextPass(reqType);
    upgAppRespPtr_->UpdateUpgAppResp(respType, resp);
}

void UpgSdk::SendAppRespFail(string str) {
    UPG_LOG_DEBUG("UpgSdk::SendAppRespFail");
    HdlrResp resp = {.resp=FAIL, .errStr=str};
    auto upgStateReqPtr = upgAppRespPtr_->GetUpgStateReqPtr();
    if (upgStateReqPtr == NULL) {
        UPG_LOG_ERROR("upgStateReqPtr is NULL");
        return;
    }
    UpgReqStateType reqType = upgStateReqPtr->upgreqstate();
    UpgStateRespType respType = upgAppRespPtr_->GetUpgAppRespNextFail(reqType);
    upgAppRespPtr_->UpdateUpgAppResp(respType, resp);
}

delphi::error UpgSdk::IsRoleAgent (SvcRole role, const char* errStr) {
    if (role != AGENT) {
        return delphi::error(errStr);
    }
    return delphi::error::OK(); 
}

delphi::objects::UpgReqPtr UpgSdk::FindUpgReqSpec(void) {
    return delphi::objects::UpgReq::FindObject(sdk_);
}

delphi::objects::UpgReqPtr UpgSdk::CreateUpgReqSpec(void) {
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(InvalidCmd);

    // add it to database
    sdk_->SetObject(req);
    return req;
}

delphi::error UpgSdk::UpdateUpgReqSpec(delphi::objects::UpgReqPtr req, UpgReqType type) {
    req->set_upgreqcmd(type);

    // add it to database
    sdk_->SetObject(req);
    return delphi::error::OK();
}

delphi::error UpgSdk::UpdateUpgReqSpec(delphi::objects::UpgReqPtr req, UpgReqType type, UpgType upgType) {
    req->set_upgreqcmd(type);
    req->set_upgreqtype(upgType);

    // add it to database
    sdk_->SetObject(req);
    return delphi::error::OK();
}

delphi::error UpgSdk::StartDisruptiveUpgrade(void) {
    return StartUpgrade(UpgTypeDisruptive);
}

delphi::error UpgSdk::StartNonDisruptiveUpgrade(void) {
    return StartUpgrade(UpgTypeNonDisruptive);
}

delphi::error UpgSdk::StartUpgrade(UpgType upgType) {
    delphi::error err = delphi::error::OK();
    UPG_LOG_DEBUG("UpgSdk::StartNonDisruptiveUpgrade");
    RETURN_IF_FAILED(IsRoleAgent(svcRole_, "Upgrade not initiated. Service is not of role AGENT."));

    delphi::objects::UpgReqPtr req = FindUpgReqSpec();
    if (req == NULL) {
        req = CreateUpgReqSpec();
    }
    UpdateUpgReqSpec(req, UpgStart, upgType);
    
    return err; 
}

delphi::error UpgSdk::AbortUpgrade(void) {
    delphi::error err = delphi::error::OK();
    UPG_LOG_DEBUG("UpgSdk::AbortUpgrade");
    RETURN_IF_FAILED(IsRoleAgent(svcRole_, "Upgrade not initiated. Service is not of role AGENT."));

    delphi::objects::UpgReqPtr req = FindUpgReqSpec();
    if (req == NULL) {
        return delphi::error("No upgrade in progress. Nothing to abort.");
    }
    UpdateUpgReqSpec(req, UpgAbort);

    return err;
}

bool UpgSdk::IsUpgradeInProgress(void) {
    UPG_LOG_DEBUG("UpgSdk::IsUpgradeInProgress");

    delphi::objects::UpgReqPtr upgReq = FindUpgReqSpec();
    if (upgReq && upgReq->upgreqcmd() == UpgStart) {
        UPG_LOG_DEBUG("Upgrade in progress");
        return true;
    }
    UPG_LOG_DEBUG("Upgrade not in progress");
    return false;
    
}

delphi::error UpgSdk::GetUpgradeStatus(vector<string>& retStr) {
    UPG_LOG_DEBUG("UpgSdk::GetUpgradeStatus");
    RETURN_IF_FAILED(IsRoleAgent(svcRole_, "Cannot get upgrade status because service is not of role Agent"));

    //Check if upgrade is initiated
    retStr.push_back("======= Checking if Upgrade is initiated =======");
    delphi::objects::UpgReqPtr upgReq = FindUpgReqSpec();
    if (upgReq == NULL) {
        retStr.push_back("No active upgrade detected from agent side.");
    } else if (upgReq->upgreqcmd() == UpgStart) {
        retStr.push_back("Agent initiated upgrade.");
    } else if (upgReq->upgreqcmd() == UpgAbort) {
        retStr.push_back("Agent aborted upgrade.");
    }

    //Get list of all applications registered with upgrade manager
    retStr.push_back("======= List of applications registered with Upgrade Manager =======");
    vector<delphi::objects::UpgAppPtr> upgAppList = delphi::objects::UpgApp::List(sdk_);
    for (vector<delphi::objects::UpgAppPtr>::iterator app=upgAppList.begin(); app!=upgAppList.end(); ++app) {
        string str = "Application " + (*app)->key() + " registered with Upgrade Manager";
        retStr.push_back(str);
    }

    //Check if Upgrade Manager is running the state machine
    retStr.push_back("======= Checking if Upgrade Manager State Machine is running =======");
    delphi::objects::UpgStateReqPtr upgStateReq = upgAppRespPtr_->GetUpgStateReqPtr();
    if (upgStateReq == NULL) {
        retStr.push_back("Upgrade Manager not running state machine");
    } else {
        retStr.push_back( "Upgrade Manager running state machine. State is:");
        retStr.push_back( StateMachine[upgStateReq->upgreqstate()].upgReqStateTypeToStr);
    }

    //Check the status of individual applications
    retStr.push_back("======= Checking status of all applications =======");
    vector<delphi::objects::UpgAppRespPtr> upgAppRespList = delphi::objects::UpgAppResp::List(sdk_);
    for (vector<delphi::objects::UpgAppRespPtr>::iterator appResp=upgAppRespList.begin(); appResp!=upgAppRespList.end(); ++appResp) {
        string str = "Application " + (*appResp)->key() + " has created response object";
        retStr.push_back(str);

        auto val = (*appResp)->upgapprespval();
        retStr.push_back(GetUpgAppRespValToStr((*appResp)->upgapprespval()));
        if (!UpgRespStatePassType(val)) {
            retStr.push_back((*appResp)->upgapprespstr());
        }
    }

    //Check if upgrade manager replied back to the agent 
    retStr.push_back("======= Checking status upgrade manager reply to agent =======");
    delphi::objects::UpgRespPtr upgResp = upgMgrAgentRespPtr_->FindUpgRespSpec();
    if (upgResp == NULL) {
        retStr.push_back( "Upgrade Manager has not replied back to agent yet.");
    } else if (upgResp->upgrespval() == UpgRespPass) {
        retStr.push_back( "Upgrade completed successfully.");
    } else if (upgResp->upgrespval() == UpgRespFail) {
        retStr.push_back( "Upgrade completed with failure.");
        for (int i=0; i<upgResp->upgrespfailstr_size(); i++) {
            retStr.push_back(upgResp->upgrespfailstr(i));
        }
    } else if (upgResp->upgrespval() == UpgRespAbort) {
        retStr.push_back( "Upgrade aborted.");
        for (int i=0; i<upgResp->upgrespfailstr_size(); i++) {
            retStr.push_back(upgResp->upgrespfailstr(i));
        }
    }
    return delphi::error::OK();
}

UpgSdk::UpgSdk(delphi::SdkPtr sk, string name, SvcRole isRoleAgent) {
    sdk_ = sk;
    svcName_ = name;
    upgAppRespPtr_ = make_shared<UpgAppRespHdlr>(sk, svcName_);
    upgReqReactPtr_ = make_shared<UpgStateReqReact>(sk, svcName_, upgAppRespPtr_);
    delphi::objects::UpgStateReq::Mount(sdk_, delphi::ReadMode);
    delphi::objects::UpgStateReq::Watch(sdk_, upgReqReactPtr_);
    sdk_->WatchMountComplete(upgReqReactPtr_);
    //TODO: make_shared do we need to free()?
    UpgAppRespPtr objUpgAppRespPtr = make_shared<delphi::objects::UpgAppResp>();
    objUpgAppRespPtr->set_key(svcName_);
    delphi::objects::UpgAppResp::MountKey(sdk_, objUpgAppRespPtr, delphi::ReadWriteMode);
    if (isRoleAgent == AGENT) {
        svcRole_ = AGENT;
        delphi::objects::UpgReq::Mount(sdk_, delphi::ReadWriteMode);
        upgAgentHandlerPtr_ = make_shared<UpgAgentHandler>();
        upgMgrAgentRespPtr_ = make_shared<UpgRespReact>(sk, upgAgentHandlerPtr_);
        delphi::objects::UpgResp::Mount(sdk_, delphi::ReadMode);
        delphi::objects::UpgResp::Watch(sdk_, upgMgrAgentRespPtr_);
        upgAppRespReactPtr_ = make_shared<UpgAppRespReact>(upgAgentHandlerPtr_);
        delphi::objects::UpgAppResp::Watch(sdk_, upgAppRespReactPtr_);
    }
    UpgAppPtr objUpgAppPtr = make_shared<delphi::objects::UpgApp>();
    objUpgAppPtr->set_key(svcName_);
    delphi::objects::UpgApp::MountKey(sdk_, objUpgAppPtr, delphi::ReadWriteMode);
    InitStateMachineVector();
}

UpgSdk::UpgSdk(delphi::SdkPtr sk, string name, SvcRole isRoleAgent, UpgAgentHandlerPtr uah) {
    sdk_ = sk;
    svcName_ = name;
    upgAppRespPtr_ = make_shared<UpgAppRespHdlr>(sk, svcName_);
    upgReqReactPtr_ = make_shared<UpgStateReqReact>(sk, svcName_, upgAppRespPtr_);
    delphi::objects::UpgStateReq::Mount(sdk_, delphi::ReadMode);
    delphi::objects::UpgStateReq::Watch(sdk_, upgReqReactPtr_);
    sdk_->WatchMountComplete(upgReqReactPtr_);
    UpgAppRespPtr objUpgAppRespPtr = make_shared<delphi::objects::UpgAppResp>();
    objUpgAppRespPtr->set_key(svcName_);
    delphi::objects::UpgAppResp::MountKey(sdk_, objUpgAppRespPtr, delphi::ReadWriteMode);
    if (isRoleAgent == AGENT) {
        svcRole_ = AGENT;
        delphi::objects::UpgReq::Mount(sdk_, delphi::ReadWriteMode);
        upgMgrAgentRespPtr_ = make_shared<UpgRespReact>(sk, uah);
        delphi::objects::UpgResp::Mount(sdk_, delphi::ReadMode);
        delphi::objects::UpgResp::Watch(sdk_, upgMgrAgentRespPtr_);
        upgAppRespReactPtr_ = make_shared<UpgAppRespReact>(uah);
        delphi::objects::UpgAppResp::Watch(sdk_, upgAppRespReactPtr_);
    }
    UpgAppPtr objUpgAppPtr = make_shared<delphi::objects::UpgApp>();
    objUpgAppPtr->set_key(svcName_);
    delphi::objects::UpgApp::MountKey(sdk_, objUpgAppPtr, delphi::ReadWriteMode);
    InitStateMachineVector();
}

UpgSdk::UpgSdk(delphi::SdkPtr sk, UpgHandlerPtr uh, string name, SvcRole isRoleAgent) {
    sdk_ = sk;
    svcName_ = name;
    upgAppRespPtr_ = make_shared<UpgAppRespHdlr>(sk, svcName_);
    upgReqReactPtr_ = make_shared<UpgStateReqReact>(sk, uh, svcName_, upgAppRespPtr_);
    delphi::objects::UpgStateReq::Mount(sdk_, delphi::ReadMode);
    delphi::objects::UpgStateReq::Watch(sdk_, upgReqReactPtr_);
    sdk_->WatchMountComplete(upgReqReactPtr_);
    UpgAppRespPtr objUpgAppRespPtr = make_shared<delphi::objects::UpgAppResp>();
    objUpgAppRespPtr->set_key(svcName_);
    delphi::objects::UpgAppResp::MountKey(sdk_, objUpgAppRespPtr, delphi::ReadWriteMode);
    if (isRoleAgent == AGENT) {
        svcRole_ = AGENT;
        delphi::objects::UpgReq::Mount(sdk_, delphi::ReadWriteMode);
        upgAgentHandlerPtr_ = make_shared<UpgAgentHandler>();
        upgMgrAgentRespPtr_ = make_shared<UpgRespReact>(sk, upgAgentHandlerPtr_);
        delphi::objects::UpgResp::Mount(sdk_, delphi::ReadMode);
        delphi::objects::UpgResp::Watch(sdk_, upgMgrAgentRespPtr_);
        upgAppRespReactPtr_ = make_shared<UpgAppRespReact>(upgAgentHandlerPtr_);
        delphi::objects::UpgAppResp::Watch(sdk_, upgAppRespReactPtr_);
    }
    UpgAppPtr objUpgAppPtr = make_shared<delphi::objects::UpgApp>();
    objUpgAppPtr->set_key(svcName_);
    delphi::objects::UpgApp::MountKey(sdk_, objUpgAppPtr, delphi::ReadWriteMode);
    InitStateMachineVector();
}

UpgSdk::UpgSdk(delphi::SdkPtr sk, UpgHandlerPtr uh, string name, SvcRole isRoleAgent, UpgAgentHandlerPtr uah) {
    sdk_ = sk;
    svcName_ = name;
    upgAppRespPtr_ = make_shared<UpgAppRespHdlr>(sk, svcName_);
    upgReqReactPtr_ = make_shared<UpgStateReqReact>(sk, uh, svcName_, upgAppRespPtr_);
    delphi::objects::UpgStateReq::Mount(sdk_, delphi::ReadMode);
    delphi::objects::UpgStateReq::Watch(sdk_, upgReqReactPtr_);
    sdk_->WatchMountComplete(upgReqReactPtr_);
    UpgAppRespPtr objUpgAppRespPtr = make_shared<delphi::objects::UpgAppResp>();
    objUpgAppRespPtr->set_key(svcName_);
    delphi::objects::UpgAppResp::MountKey(sdk_, objUpgAppRespPtr, delphi::ReadWriteMode);
    if (isRoleAgent == AGENT) {
        svcRole_ = AGENT;
        delphi::objects::UpgReq::Mount(sdk_, delphi::ReadWriteMode);
        upgMgrAgentRespPtr_ = make_shared<UpgRespReact>(sk, uah);
        delphi::objects::UpgResp::Mount(sdk_, delphi::ReadMode);
        delphi::objects::UpgResp::Watch(sdk_, upgMgrAgentRespPtr_);
        upgAppRespReactPtr_ = make_shared<UpgAppRespReact>(uah);
        delphi::objects::UpgAppResp::Watch(sdk_, upgAppRespReactPtr_);
    }
    UpgAppPtr objUpgAppPtr = make_shared<delphi::objects::UpgApp>();
    objUpgAppPtr->set_key(svcName_);
    delphi::objects::UpgApp::MountKey(sdk_, objUpgAppPtr, delphi::ReadWriteMode);
    InitStateMachineVector();
}

} // namespace upgrade
