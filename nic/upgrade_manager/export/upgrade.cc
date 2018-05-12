// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade.hpp"

namespace upgrade {

using namespace std;

void UpgHandler::UpgReqStatusCreate(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler UpgReqStatusCreate called!!");
}

// OnUpgReqStatusCreate gets called when UpgReqStatus object is created
delphi::error UpgReqReactor::OnUpgReqStatusCreate(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgReqStatus got created for {}/{}", req, req->meta().ShortDebugString());
    //create the object
    if (this->upgHdlrPtr_)
        this->upgHdlrPtr_->UpgReqStatusCreate(req);
    return delphi::error::OK();
}

// OnUpgReqStatusDelete gets called when UpgReqStatus object is deleted
delphi::error UpgReqReactor::OnUpgReqStatusDelete(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgReqStatus got deleted");
    //delete the object
    return delphi::error::OK();
}

// OnUpgReqState gets called when UpgReqState attribute changes
delphi::error UpgReqReactor::OnUpgReqState(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("alok");
    // start or abort?
    if (req->upgreqstate() == upgrade::UpgReqRcvd) {
        LogInfo("Upgrade Request Received");
    } else {
        LogInfo("Upgrade Request");
    }
    //switch and invoke callback
    return delphi::error::OK();
}

void UpgSdk::OnMountComplete(void) {
    LogInfo("UpgReqStatusMgr OnMountComplete called");

    vector<delphi::objects::UpgReqStatusPtr> upgReqStatuslist = delphi::objects::UpgReqStatus::List(sdk_);
    for (vector<delphi::objects::UpgReqStatusPtr>::iterator reqStatus=upgReqStatuslist.begin(); reqStatus != upgReqStatuslist.end(); ++reqStatus) {
        this->upgReqReactPtr_->OnUpgReqStatusCreate(*reqStatus);
    }
}

} // namespace upgrade
