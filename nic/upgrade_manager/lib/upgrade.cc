// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade.hpp"

namespace upgrade {

using namespace std;

// OnInterfaceSpecCreate gets called when InterfaceSpec object is created
delphi::error UpgradeMgr::OnUpgradeRequestSpecCreate(delphi::objects::UpgradeRequestSpecPtr req) {
    LogInfo("UpgradeRequestSpec got created for {}/{}", req, req->meta().ShortDebugString());

    // find the status object
    auto upgReqStatus = this->findUpgReqStatus(req->key().id());
    if (upgReqStatus == NULL) {
        // create it since it doesnt exist
        RETURN_IF_FAILED(this->createUpgReqStatus(req->key().id(), upgrade::RcvdUpgReq));
    }

    return delphi::error::OK();
}

// OnInterfaceSpecDelete gets called when InterfaceSpec object is deleted
delphi::error UpgradeMgr::OnUpgradeRequestSpecDelete(delphi::objects::UpgradeRequestSpecPtr req) {
    LogInfo("UpgradeRequestSpec got deleted");
    auto upgReqStatus = this->findUpgReqStatus(req->key().id());
    if (upgReqStatus != NULL) {
        LogInfo("Deleting Upgrade Request Status");
        sdk_->DeleteObject(upgReqStatus);
    }
    return delphi::error::OK();
}

// OnUpgReqAction gets called when UpgReqAction attribute changes
delphi::error UpgradeMgr::OnUpgReqAction(delphi::objects::UpgradeRequestSpecPtr req) {
    // start or abort?
    if (req->upgreqaction() == upgrade::UpgStart) {
        LogInfo("Start Upgrade");
    } else {
        LogInfo("Abort Upgrade");
    }

    // set the oper state on status object
    auto upgReqStatus = this->findUpgReqStatus(req->key().id());
    if (upgReqStatus != NULL) {
        LogInfo("Updated Upgrade Request Status");
        upgReqStatus->set_upgreqstatus(upgrade::RcvdUpgReq);
        sdk_->SetObject(upgReqStatus);
    }

    return delphi::error::OK();
}

// createIntfStatus creates a interface status object
delphi::error UpgradeMgr::createUpgReqStatus(uint32_t id, upgrade::UpgReqState status) {
    // create an object
    delphi::objects::UpgReqStatusPtr req = make_shared<delphi::objects::UpgReqStatus>();
    req->set_key(id);
    req->set_upgreqstatus(status);

    // add it to database
    sdk_->SetObject(req);

    LogInfo("Created upgrade request status object for id {} state {} req: {}", id, status, req);

    return delphi::error::OK();
}

// findUpgReqStatus finds the upgrade request status object
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
    delphi::objects::UpgradeRequestSpec::Mount(sdk_, delphi::ReadWriteMode);
    delphi::objects::UpgReqStatus::Mount(sdk_, delphi::ReadWriteMode);

    // create interface event handler
    upgMgr_ = make_shared<UpgradeMgr>(sdk_);

    // Register interface reactor
    delphi::objects::UpgradeRequestSpec::Watch(sdk_, upgMgr_);

    LogInfo("Upgrade service constructor got called");
}

// OnMountComplete gets called when all the objects are mounted
void UpgradeService::OnMountComplete() {
    string out_str;

    LogInfo("UpgradeService OnMountComplete got called\n");

    // walk all interface objects and reconcile them
    vector<delphi::objects::UpgradeRequestSpecPtr> upgReqlist = delphi::objects::UpgradeRequestSpec::List(sdk_);
    for (vector<delphi::objects::UpgradeRequestSpecPtr>::iterator req=upgReqlist.begin(); req!=upgReqlist.end(); ++req) {
        upgMgr_->OnUpgradeRequestSpecCreate(*req);
    }

    LogInfo("============== UpgradeService Finished Reconciliation ==================\n");
}

// createUpgReqSpec creates a dummy upgrade request 
void UpgradeService::createUpgReqSpec() {
    // create an object
    delphi::objects::UpgradeRequestSpecPtr req = make_shared<delphi::objects::UpgradeRequestSpec>();
    srand(time(NULL));
    req->mutable_key()->set_id(rand());
    req->set_upgreqaction(upgrade::InvalidAction);

    // add it to database
    sdk_->SetObject(req);

    req->set_upgreqaction(upgrade::UpgStart);
    sdk_->SetObject(req);
}

// createTimerHandler creates a dummy code upgrade request
void UpgradeService::createTimerHandler(ev::timer &watcher, int revents) {
    LogInfo("Creating a dummy upgrade request");

    // create a dummy upgrade request
    this->createUpgReqSpec();
}
} // namespace upgrade
