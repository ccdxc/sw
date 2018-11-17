// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include "nic/sdk/include/sdk/thread.hpp"
#include "platform/src/lib/sysmgr/sysmgr.hpp"
#include "platform/src/lib/upgrade/upgrade.hpp"

#include "delphic.hpp"

#define NICMGR_DELPHI_REG_THREAD_ID 1

using namespace std;
using namespace nicmgr;
using namespace upgrade;

// this method is called from the pthread_create
void *nicmgr_delphi_client_entry (void *ctxt)
{
    printf("Delphi thread starting ...\n");
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());

    g_nicmgr_svc = make_shared<NicMgrService>(sdk);

    // Register NicMgr as Delphi Service
    sdk->RegisterService(g_nicmgr_svc);

    // init port status handler
    init_port_status_handler(sdk);

    // init hal status handler
    init_hal_status_handler(sdk);

    // init accel dev related handlers
    init_accel_objects(sdk);

    //  connect to delphi
    sdk->Connect();
    return NULL;
}
