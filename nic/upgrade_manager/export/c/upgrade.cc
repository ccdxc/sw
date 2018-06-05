// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade.hpp"

namespace upgrade {

using namespace std;

void UpgSdk::OnMountComplete(void) {
    LogInfo("UpgStateReqMgr OnMountComplete called");

    vector<delphi::objects::UpgStateReqPtr> upgReqStatuslist = delphi::objects::UpgStateReq::List(sdk_);
    for (vector<delphi::objects::UpgStateReqPtr>::iterator reqStatus=upgReqStatuslist.begin(); reqStatus != upgReqStatuslist.end(); ++reqStatus) {
        upgReqReactPtr_->OnUpgStateReqCreate(*reqStatus);
    }
}

void UpgSdk::SendAppRespSuccess(void) {
    LogInfo("Application returning success via UpgSdk");
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    auto upgStateReqPtr = upgAppRespPtr_->GetUpgStateReqPtr();
    if (upgStateReqPtr == NULL) {
        LogInfo("upgStateReqPtr is NULL");
        return;
    }
    UpgReqStateType reqType = upgStateReqPtr->upgreqstate(); 
    UpgRespStateType respType = upgAppRespPtr_->GetUpgAppRespNextPass(reqType);
    upgAppRespPtr_->UpdateUpgAppResp(respType, resp);
}

void UpgSdk::SendAppRespFail(string str) {
    LogInfo("UpgSdk::SendAppRespFail");
    HdlrResp resp = {.resp=FAIL, .errStr=str};
    auto upgStateReqPtr = upgAppRespPtr_->GetUpgStateReqPtr();
    if (upgStateReqPtr == NULL) {
        LogInfo("upgStateReqPtr is NULL");
        return;
    }
    UpgReqStateType reqType = upgStateReqPtr->upgreqstate();
    UpgRespStateType respType = upgAppRespPtr_->GetUpgAppRespNextFail(reqType);
    upgAppRespPtr_->UpdateUpgAppResp(respType, resp);
}

delphi::error UpgSdk::IsRoleAgent (SvcRole role, const char* errStr) {
    if (role != AGENT) {
        return delphi::error(errStr);
    }
    return delphi::error::OK(); 
}

delphi::objects::UpgReqPtr UpgSdk::FindUpgReqSpec(void) {
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);

    // find the object
    delphi::BaseObjectPtr obj = sdk_->FindObject(req);

    return static_pointer_cast<delphi::objects::UpgReq>(obj);
}

delphi::objects::UpgReqPtr UpgSdk::CreateUpgReqSpec(void) {
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);
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

delphi::error UpgSdk::StartUpgrade(void) {
    delphi::error err = delphi::error::OK();
    LogInfo("UpgSdk::StartUpgrade");
    RETURN_IF_FAILED(IsRoleAgent(svcRole_, "Upgrade not initiated. Service is not of role AGENT."));

    delphi::objects::UpgReqPtr req = FindUpgReqSpec();
    if (req == NULL) {
        req = CreateUpgReqSpec();
    }
    UpdateUpgReqSpec(req, UpgStart);
    
    return err; 
}

delphi::error UpgSdk::AbortUpgrade(void) {
    delphi::error err = delphi::error::OK();
    LogInfo("UpgSdk::AbortUpgrade");
    RETURN_IF_FAILED(IsRoleAgent(svcRole_, "Upgrade not initiated. Service is not of role AGENT."));

    delphi::objects::UpgReqPtr req = FindUpgReqSpec();
    if (req == NULL) {
        return delphi::error("No upgrade in progress. Nothing to abort.");
    }
    UpdateUpgReqSpec(req, UpgAbort);

    return err;
}

bool UpgSdk::IsUpgradeInProgress(void) {
    LogInfo("UpgSdk::IsUpgradeInProgress");

    delphi::objects::UpgReqPtr upgReq = FindUpgReqSpec();
    if (upgReq && upgReq->upgreqcmd() == UpgStart) {
        LogInfo("Upgrade in progress");
        return true;
    }
    LogInfo("Upgrade not in progress");
    return false;
    
}

delphi::error UpgSdk::GetUpgradeStatus(vector<string>& retStr) {
    LogInfo("UpgSdk::GetUpgradeStatus");
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

        uint32_t val = (*appResp)->upgapprespval();
        bool pass = (val % 2 == 0);
        retStr.push_back((pass)?StateMachine[val/2].upgAppRespValToStrPass:StateMachine[val/2].upgAppRespValToStrFail);
        if (!pass) {
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
    upgAppRespPtr_ = make_shared<UpgAppRespHdlr>(sk, name);
    upgReqReactPtr_ = make_shared<UpgReqReactor>(sk, name, upgAppRespPtr_);
    delphi::objects::UpgStateReq::Mount(sdk_, delphi::ReadMode);
    delphi::objects::UpgStateReq::Watch(sdk_, upgReqReactPtr_);
    sdk_->MountKey("UpgAppResp", name, delphi::ReadWriteMode);
    //delphi::objects::UpgAppResp::MountKey(sdk_, name, delphi::ReadWriteMode);
    if (isRoleAgent == AGENT) {
        svcRole_ = AGENT;
        delphi::objects::UpgReq::Mount(sdk_, delphi::ReadWriteMode);
        upgAgentHandlerPtr_ = make_shared<UpgAgentHandler>();
        upgMgrAgentRespPtr_ = make_shared<UpgMgrAgentRespReact>(sk, upgAgentHandlerPtr_);
        delphi::objects::UpgResp::Mount(sdk_, delphi::ReadMode);
        delphi::objects::UpgResp::Watch(sdk_, upgMgrAgentRespPtr_);
        upgAppRespReactPtr_ = make_shared<UpgAppRespReact>(upgAgentHandlerPtr_);
        delphi::objects::UpgAppResp::Watch(sdk_, upgAppRespReactPtr_);
    }
    InitStateMachineVector();
}

UpgSdk::UpgSdk(delphi::SdkPtr sk, string name, SvcRole isRoleAgent, UpgAgentHandlerPtr uah) {
    sdk_ = sk;
    svcName_ = name;
    upgAppRespPtr_ = make_shared<UpgAppRespHdlr>(sk, name);
    upgReqReactPtr_ = make_shared<UpgReqReactor>(sk, name, upgAppRespPtr_);
    delphi::objects::UpgStateReq::Mount(sdk_, delphi::ReadMode);
    delphi::objects::UpgStateReq::Watch(sdk_, upgReqReactPtr_);
    sdk_->MountKey("UpgAppResp", name, delphi::ReadWriteMode);
    //delphi::objects::UpgAppResp::MountKey(sdk_, name, delphi::ReadWriteMode);
    if (isRoleAgent == AGENT) {
        svcRole_ = AGENT;
        delphi::objects::UpgReq::Mount(sdk_, delphi::ReadWriteMode);
        upgMgrAgentRespPtr_ = make_shared<UpgMgrAgentRespReact>(sk, uah);
        delphi::objects::UpgResp::Mount(sdk_, delphi::ReadMode);
        delphi::objects::UpgResp::Watch(sdk_, upgMgrAgentRespPtr_);
        upgAppRespReactPtr_ = make_shared<UpgAppRespReact>(uah);
        delphi::objects::UpgAppResp::Watch(sdk_, upgAppRespReactPtr_);
    }
    InitStateMachineVector();
}

UpgSdk::UpgSdk(delphi::SdkPtr sk, UpgHandlerPtr uh, string name, SvcRole isRoleAgent) {
    sdk_ = sk;
    svcName_ = name;
    upgAppRespPtr_ = make_shared<UpgAppRespHdlr>(sk, name);
    upgReqReactPtr_ = make_shared<UpgReqReactor>(sk, uh, name, upgAppRespPtr_);
    delphi::objects::UpgStateReq::Mount(sdk_, delphi::ReadMode);
    delphi::objects::UpgStateReq::Watch(sdk_, upgReqReactPtr_);
    sdk_->MountKey("UpgAppResp", name, delphi::ReadWriteMode);
    //delphi::objects::UpgAppResp::MountKey(sdk_, name, delphi::ReadWriteMode);
    if (isRoleAgent == AGENT) {
        svcRole_ = AGENT;
        delphi::objects::UpgReq::Mount(sdk_, delphi::ReadWriteMode);
        upgAgentHandlerPtr_ = make_shared<UpgAgentHandler>();
        upgMgrAgentRespPtr_ = make_shared<UpgMgrAgentRespReact>(sk, upgAgentHandlerPtr_);
        delphi::objects::UpgResp::Mount(sdk_, delphi::ReadMode);
        delphi::objects::UpgResp::Watch(sdk_, upgMgrAgentRespPtr_);
        upgAppRespReactPtr_ = make_shared<UpgAppRespReact>(upgAgentHandlerPtr_);
        delphi::objects::UpgAppResp::Watch(sdk_, upgAppRespReactPtr_);
    }
    InitStateMachineVector();
}

UpgSdk::UpgSdk(delphi::SdkPtr sk, UpgHandlerPtr uh, string name, SvcRole isRoleAgent, UpgAgentHandlerPtr uah) {
    sdk_ = sk;
    svcName_ = name;
    upgAppRespPtr_ = make_shared<UpgAppRespHdlr>(sk, name);
    upgReqReactPtr_ = make_shared<UpgReqReactor>(sk, uh, name, upgAppRespPtr_);
    delphi::objects::UpgStateReq::Mount(sdk_, delphi::ReadMode);
    delphi::objects::UpgStateReq::Watch(sdk_, upgReqReactPtr_);
    sdk_->MountKey("UpgAppResp", name, delphi::ReadWriteMode);
    //delphi::objects::UpgAppResp::MountKey(sdk_, name, delphi::ReadWriteMode);
    if (isRoleAgent == AGENT) {
        svcRole_ = AGENT;
        delphi::objects::UpgReq::Mount(sdk_, delphi::ReadWriteMode);
        upgMgrAgentRespPtr_ = make_shared<UpgMgrAgentRespReact>(sk, uah);
        delphi::objects::UpgResp::Mount(sdk_, delphi::ReadMode);
        delphi::objects::UpgResp::Watch(sdk_, upgMgrAgentRespPtr_);
        upgAppRespReactPtr_ = make_shared<UpgAppRespReact>(uah);
        delphi::objects::UpgAppResp::Watch(sdk_, upgAppRespReactPtr_);
    }
    InitStateMachineVector();
}

} // namespace upgrade
