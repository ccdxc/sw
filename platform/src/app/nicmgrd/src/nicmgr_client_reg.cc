// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <upgrade/upgrade.hpp>
#include <delphi/delphi.hpp>
#include <sysmgr/sysmgr.hpp>
#include "nic/sdk/include/sdk/thread.hpp"

#define NICMGR_DELPHI_REG_THREAD_ID 1

using namespace std;
using namespace nicmgr;
using namespace upgrade;

namespace nicmgr {

void *nicmgr_delphi_client_entry(void *ctxt);

void nicmgr_do_client_registration(void)
{
    sdk::lib::thread *delphi_thread; 
    tid = sdk::lib::thread::factory("delphi-registration", NICMGR_DELPHI_REG_THREAD_ID, 
                              sdk::lib::THREAD_ROLE_CONTROL, 0x0, 
                              nicmgr_delphi_client_entry,
                              sched_get_priority_max(SCHED_OTHER), 
                              SCHED_OTHER, NULL);
   if (!delphi_thread) {
   // ASSERT
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

    shared_ptr<NicMgrService> nicmgr_svc = make_shared<NicMgrService>(sdk, svc_name);

    // Sysmgr registration here ??
    //
    //
    // Register NicMgr as Delphi Service
    sdk->RegisterService(nicmgr_svc);

    //  Event loop 
    return sdk->MainLooop();
}
    
} //namespace nicmgr

