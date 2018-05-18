// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "nic/upgrade_manager/example/nmd.hpp"

using namespace std;
using namespace nmd;

int main(int argc, char **argv) {
    // Create delphi SDK
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    string myName = "NMD";

    // Create a service instance
    shared_ptr<NMDService> exupgsvc = make_shared<NMDService>(sdk, myName);
    assert(exupgsvc != NULL);

    sdk->RegisterService(exupgsvc);

    // start a timer to create an object
    exupgsvc->createTimer.set<NMDService, &NMDService::createTimerHandler>(exupgsvc.get());
    exupgsvc->createTimer.start(2, 2);

    // run the main loop
    return sdk->MainLoop();
}

namespace nmd {

delphi::error NMDUpgAppRespHdlr::OnUpgAppRespCreate(delphi::objects::UpgAppRespPtr resp) {
    LogInfo("NMDUpgAppRespHdlr::OnUpgAppRespCreate called");
    return delphi::error::OK();
}

string GetAppRespStr(delphi::objects::UpgAppRespPtr resp) {
    switch (resp->upgapprespval()) {
        case upgrade::PreUpgStatePass:
            return ("Pre upgrade checks passed");
        case upgrade::ProcessesQuiescedPass:
            return ("Processes Quiesce end");
        case upgrade::PostBinRestartPass:
            return ("Binaries Restarted");
        case upgrade::DataplaneDowntimeStartPass:
            return ("Dataplane downtime end");
        case upgrade::CleanupPass:
            return ("Cleanup finished");
        default:
            return ("");
    }
}

delphi::error NMDUpgAppRespHdlr::OnUpgAppRespVal(delphi::objects::UpgAppRespPtr resp) {
    if (GetAppRespStr(resp) != "")
        LogInfo("NMDUpgAppRespHdlr::OnUpgAppRespVal called: {}", GetAppRespStr(resp));
    return delphi::error::OK();
}



int count = 0;
// NMDService constructor
NMDService::NMDService(delphi::SdkPtr sk) : NMDService(sk, "NMDService") {
}

NMDService::NMDService(delphi::SdkPtr sk, string name) {
    this->sdk_ = sk;
    this->svcName_ = name;
    delphi::objects::UpgReq::Mount(sdk_, delphi::ReadWriteMode);
    delphi::objects::UpgAppResp::Mount(sdk_, delphi::ReadMode);

    nmdUpgAppRespHdlr_ = make_shared<NMDUpgAppRespHdlr>(sdk_);

    delphi::objects::UpgAppResp::Watch(sdk_, nmdUpgAppRespHdlr_);
    LogInfo("NMD service constructor got called");
}

//createUpgReqSpec creates a dummy upgrade request
void NMDService::createUpgReqSpec() {
    // create an object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);
    req->set_upgreqcmd(upgrade::InvalidCmd);

    // add it to database
    sdk_->SetObject(req);
}

void NMDService::updateUpgReqSpec() {
    // update an object

    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);

    // find the object
    delphi::BaseObjectPtr obj = sdk_->FindObject(req);
    
    req = static_pointer_cast<delphi::objects::UpgReq>(obj);

    req->set_upgreqcmd(upgrade::UpgStart);

    // add it to database
    sdk_->SetObject(req);
}



// createTimerHandler creates a dummy code upgrade request
void NMDService::createTimerHandler(ev::timer &watcher, int revents) {

    // create a dummy upgrade request
    if (count == 0) {
        LogInfo("NMD: Process came up");
        this->createUpgReqSpec();
        count++;
    } else if (count == 1) {
        count++;
        LogInfo("NMD: start upgrade");
        this->updateUpgReqSpec();
    }
}

// createTimerHandler creates a dummy code upgrade request
void NMDService::createTimerUpdHandler(ev::timer &watcher, int revents) {
    LogInfo("Creating a update upgrade request");

    // create a dummy upgrade request
    this->updateUpgReqSpec();
}

void NMDService::OnMountComplete() {
}

}
