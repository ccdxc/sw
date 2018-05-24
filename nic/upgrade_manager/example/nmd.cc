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
    exupgsvc->createTimer.start(2, 0);

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

    upgsdk_ = make_shared<UpgSdk>(sdk_, make_shared<NMDSvcHandler>(), name, AGENT);

    LogInfo("NMD service constructor got called");
}

// createTimerHandler creates a dummy code upgrade request
void NMDService::createTimerHandler(ev::timer &watcher, int revents) {
    this->upgsdk_->StartUpgrade();
    LogInfo("NMD: called start upgrade");
}

void NMDService::OnMountComplete() {
}

}
