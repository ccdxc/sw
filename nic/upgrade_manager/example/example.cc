// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "example.hpp"

namespace example {

using namespace std;
using namespace upgrade;

// ExUpgSvc constructor
ExUpgSvc::ExUpgSvc(delphi::SdkPtr sk, string name) {
    // save a pointer to sdk
    this->sdk_ = sk;
    this->svcName_ = name;

    // create upgrade request status manager event handler
    upgsdk_ = make_shared<UpgSdk>(sdk_, make_shared<ExSvcHandler>(), name);

    LogInfo("Example upgrade service constructor got called");
}

// createTimerHandler creates a dummy code upgrade request
void ExUpgSvc::createTimerHandler(ev::timer &watcher, int revents) {
    LogInfo("Processing finished by application. Going to ask UpgSdk to respond with success.");
    this->upgsdk_->SendAppRespSuccess();
}
} // namespace example 
