// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <nicmgr_upgrade.hpp>
#include <nicmgr_delphi_client.hpp>
#include <nicmgr_sysmgr.hpp>
#include "nic/sdk/include/sdk/thread.hpp"
#include "dev.hpp"
#include "logger.hpp"

#define NICMGR_DELPHI_REG_THREAD_ID 1

using namespace std;
using namespace nicmgr;
using namespace upgrade;

sdk::lib::thread *g_nicmgr_threads[NICMGR_THREAD_ID_MAX];
void *nicmgr_delphi_client_entry(void *ctxt);

void nicmgr_do_client_registration (void)
{
    pthread_t ev_thread_id = 0;

    // create a thread for delphi events
    pthread_create(&ev_thread_id, 0, &nicmgr_delphi_client_entry, (void*)NULL);
    printf("\nInstantiated delphi thread ...\n");
}

// this method is called from the pthread_create
void *nicmgr_delphi_client_entry (void *ctxt)
{
    printf("Delphi thread starting ...\n");
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());

    nicmgr_svc_ = make_shared<NicMgrService>(sdk);

    // Register NicMgr as Delphi Service
    sdk->RegisterService(nicmgr_svc_);

    // init port reactor
    init_port_reactors(sdk);

    printf("Delphi thread entering main loop ...\n");

    //  Event loop 
    sdk->MainLoop();
    return NULL;
}
