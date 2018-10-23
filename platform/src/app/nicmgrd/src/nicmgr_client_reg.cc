// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <nicmgr_upgrade.hpp>
#include <nicmgr_delphi_client.hpp>
#include <nicmgr_sysmgr.hpp>
#include "nic/sdk/include/sdk/thread.hpp"
#include "dev.hpp"

#define NICMGR_DELPHI_REG_THREAD_ID 1

using namespace std;
using namespace nicmgr;
using namespace upgrade;


sdk::lib::thread *g_nicmgr_threads[NICMGR_THREAD_ID_MAX];

void *nicmgr_delphi_client_entry(void *ctxt);

void nicmgr_do_client_registration(void)
{
    sdk::lib::thread *delphi_thread; 
    delphi_thread = sdk::lib::thread::factory("delphi-registration", NICMGR_DELPHI_REG_THREAD_ID, 
                              sdk::lib::THREAD_ROLE_CONTROL, 0x0, 
                              nicmgr_delphi_client_entry,
                              sched_get_priority_max(SCHED_OTHER), 
                              SCHED_OTHER, NULL);
   if (!delphi_thread) {
   // ASSERT
   } else {
       g_nicmgr_threads[NICMGR_DELPHI_REG_THREAD_ID] = delphi_thread;
   }

   delphi_thread->set_data(delphi_thread);
   delphi_thread->set_pthread_id(pthread_self());
   delphi_thread->set_running(true);
}

//This method is called from the pthread_create
//
void *nicmgr_delphi_client_entry(void *ctxt)
{
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    string svc_name = "NicMgrService";

    nicmgr_svc_ = make_shared<NicMgrService>(sdk, svc_name);

    // Sysmgr registration here ??
    //
    //
    // Register NicMgr as Delphi Service
    sdk->RegisterService(nicmgr_svc_);

    //  Event loop 
    sdk->MainLoop();
    return NULL;
}
    

