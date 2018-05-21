// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "hal_svc.hpp"

using namespace std;
using hal::svc::HalService;

// main function
int main(int argc, char **argv) {
    // Create delphi SDK
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());

    // Create a service instance
    shared_ptr<HalService> halsvc = make_shared<HalService>(sdk);
    assert(halsvc != NULL);
    sdk->RegisterService(halsvc);

    // run the main loop
    return sdk->MainLoop();
}
