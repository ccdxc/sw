// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include "delphi_sdk.hpp"

// StartTestLoop starts the test loop in a pthread
void * StartTestLoop(void* arg) {
    delphi::SdkPtr sdk = *(delphi::SdkPtr *)arg;

    // run the main loop
    sdk->TestLoop();

    return NULL;
}
