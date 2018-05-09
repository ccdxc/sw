// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade.hpp"

using namespace std;
using namespace upgrade;

int main(int argc, char **argv) {
    // Create delphi SDK
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    string myName = "UpgradeService";

    // if name is specified, use it
    if (argc > 1) {
        myName = argv[1];
    }

    // Create a service instance
    shared_ptr<UpgradeService> upgsvc = make_shared<UpgradeService>(sdk, myName);
    assert(upgsvc != NULL);
    sdk->RegisterService(upgsvc);

    // start a timer to create an object
    upgsvc->createTimer.set<UpgradeService, &UpgradeService::createTimerHandler>(upgsvc.get());
    upgsvc->createTimer.start(0.5, 0.0);

    // run the main loop
    return sdk->MainLoop();
}
