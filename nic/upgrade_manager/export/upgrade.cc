// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade.hpp"

namespace upgrade {

using namespace std;

// OnUpgReqStatusCreate gets called when UpgReqStatus object is created
delphi::error UpgReqStatusMgr::OnUpgReqStatusCreate(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgReqStatus got created for {}/{}", req, req->meta().ShortDebugString());
    //create the object
    return delphi::error::OK();
}

// OnUpgReqStatusDelete gets called when UpgReqStatus object is deleted
delphi::error UpgReqStatusMgr::OnUpgReqStatusDelete(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgReqStatus got deleted");
    //delete the object
    return delphi::error::OK();
}

// OnUpgReqState gets called when UpgReqState attribute changes
delphi::error UpgReqStatusMgr::OnUpgReqState(delphi::objects::UpgReqStatusPtr req) {
    // start or abort?
    if (req->upgreqstate() == upgrade::UpgReqRcvd) {
        LogInfo("Upgrade Request Received");
    } else {
        LogInfo("Upgrade Request");
    }
    //switch and invoke callback
    return delphi::error::OK();
}

} // namespace upgrade
