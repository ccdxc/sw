//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "nic/sdk/include/sdk/types.hpp"
#include "lib/logger/logger.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/metaswitch/include/pds_cfg.hpp"
#include "nic/metaswitch/thread/pds.hpp"

namespace pds {

//------------------------------------------------------------------------------
// handler invoked during normal termnination of HAL (e.g., gtests)
//------------------------------------------------------------------------------
static void
pdsa_atexit_handler (void)
{
    HAL_TRACE_DEBUG("Flushing logs before exiting ...");
}

//------------------------------------------------------------------------------
// initialize all the signal handlers
//------------------------------------------------------------------------------
static void
pdsa_sig_handler (int sig, siginfo_t *info, void *ptr)
{
    cout << "HAL received signal " << sig;

    switch (sig) {
    case SIGINT:
    case SIGTERM:
    case SIGQUIT:
        raise(SIGKILL);
        break;

    case SIGUSR1:
    case SIGUSR2:
        break;

    case SIGHUP:
    case SIGCHLD:
    case SIGURG:
    default:
        break;
    }
}

//------------------------------------------------------------------------------
// bring up nbase thread
//------------------------------------------------------------------------------
static bool
pdsa_nbase_thread_init ()
{
    sdk::lib::thread    *nbase_thread;

    nbase_thread =
        pdsa_thread_create(std::string("nbase").c_str(),
            PDS_THREAD_ID_NBASE,
            sdk::lib::THREAD_ROLE_CONTROL,
            0x0,    // use all control cores
            svc::nbase_start,
            sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            NULL);
    SDK_ASSERT_TRACE_RETURN((nbase_thread != NULL), HAL_RET_ERR,
                            "Failed to spawn nbase thread");
    nbase_thread->start(nbase_thread);
    return true;
}

//------------------------------------------------------------------------------
// init function for PDSA
//------------------------------------------------------------------------------
int
pdsa_init (void)
{
    int ret = 0;
    // do SDK initialization, if any
    //hal_sdk_init();

    // instantiate nbase thread
    pdsa_nbase_thread_init();


    // install signal handlers
    pdsa_sig_init(pdsa_sig_handler);

    // install atexit() handler
    atexit(pdsa_atexit_handler);

    return ret;
}

}    // namespace pds
