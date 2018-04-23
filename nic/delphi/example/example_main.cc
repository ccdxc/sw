// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "example.hpp"

using namespace std;
using namespace example;

int main(int argc, char **argv) {
    // Create delphi SDK
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    string myName = "ExampleService";

    // if name is specified, use it
    if (argc > 1) {
        myName = argv[1];
    }

    // Create a service instance
    shared_ptr<ExampleService> exsvc = make_shared<ExampleService>(sdk, myName);
    assert(exsvc != NULL);
    sdk->RegisterService(exsvc);

    // start a timer to create an object
    exsvc->createTimer.set<ExampleService, &ExampleService::createTimerHandler>(exsvc.get());
    exsvc->createTimer.start(0.5, 0.0);

    // run the main loop
    return sdk->MainLoop();
}
