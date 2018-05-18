// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "nic/upgrade_manager/example/example.hpp"

using namespace std;
using namespace upgrade;
using namespace example;

int main(int argc, char **argv) {
    // Create delphi SDK
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    string myName = "ExampleUpgradeService";

    // Create a service instance
    shared_ptr<ExUpgSvc> exupgsvc = make_shared<ExUpgSvc>(sdk, myName);
    assert(exupgsvc != NULL);

    sdk->RegisterService(exupgsvc);

    // start a timer to create an object
    exupgsvc->createTimer.set<ExUpgSvc, &ExUpgSvc::createTimerHandler>(exupgsvc.get());
    exupgsvc->createTimer.start(15, 5);

    // run the main loop
    return sdk->MainLoop();
}
