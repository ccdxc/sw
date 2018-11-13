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

    // init port reactor
    init_port_reactors(sdk);

    //  connect to delphi
    sdk->Connect();
    return NULL;
}

void nicmgr_do_client_registration (void)
{
    pthread_t ev_thread_id = 0;

    // create a thread for delphi events
    pthread_create(&ev_thread_id, 0, nicmgr_delphi_client_entry, (void*)NULL);
    printf("\nInstantiated delphi thread ...\n");
}

