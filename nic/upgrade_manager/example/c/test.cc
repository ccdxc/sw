// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "test.hpp"

namespace test {

using namespace std;
using namespace upgrade;

// TestUpgSvc constructor
TestUpgSvc::TestUpgSvc(delphi::SdkPtr sk, string name) {
    // save a pointer to sdk
    sdk_ = sk;
    svcName_ = name;

    upgsdk_ = make_shared<UpgSdk>(sdk_, make_shared<ExSvcHandler>(), name, NON_AGENT, nullptr);

    UPG_LOG_DEBUG("Example upgrade service constructor got called");
}

// createTimerHandler creates a dummy code upgrade request
void TestUpgSvc::createTimerHandler(ev::timer &watcher, int revents) {
    upgsdk_->SendAppRespSuccess();
}

void TestUpgSvc::unRegUpgTimerHandler(ev::timer &watcher, int revents) {
    UPG_LOG_DEBUG("TestUpgSvc Unregistering.");
    upgsdk_->UnRegUpg();
}
} // namespace test 
