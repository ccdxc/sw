// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "nic/upgrade_manager/example/nmd.hpp"

using namespace std;
using namespace nmd;
using namespace upgrade;

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

int count = 0;
// NMDService constructor
NMDService::NMDService(delphi::SdkPtr sk) : NMDService(sk, "NMDService") {
}

NMDService::NMDService(delphi::SdkPtr sk, string name) {
    this->sdk_ = sk;
    this->svcName_ = name;
    delphi::objects::UpgReq::Mount(sdk_, delphi::ReadWriteMode);
    delphi::objects::UpgAppResp::Mount(sdk_, delphi::ReadMode);

    upgsdk_ = make_shared<UpgSdk>(sdk_, make_shared<NMDSvcHandler>(), name, AGENT);

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
