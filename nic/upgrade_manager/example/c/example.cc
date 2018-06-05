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
    sdk_ = sk;
    svcName_ = name;

    upgsdk_ = make_shared<UpgSdk>(sdk_, make_shared<ExSvcHandler>(), name, NON_AGENT);

    LogInfo("Example upgrade service constructor got called");
}

// createTimerHandler creates a dummy code upgrade request
void ExUpgSvc::createTimerHandler(ev::timer &watcher, int revents) {
    LogInfo("Processing finished by application. Going to ask UpgSdk to respond with success.");
    upgsdk_->SendAppRespSuccess();
}
} // namespace example 
