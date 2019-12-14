//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Main entry point for the Pensando Distributed Services Agent (PDSA)
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pdsa_state_init.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_init.hpp"
#include "nic/metaswitch/stubs/pdsa_stubs_init.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/sdk/include/sdk/base.hpp"

namespace pdsa_stub {

int pdsa_init ()
{
    if (!pdsa_stub::state_init()) {
        return -1;
    }
    if (!pdsa_stub_mgmt_init()) {
        goto error;
    }

    return 0;

error:
    pdsa_stub::state_destroy();
    return -1;
}

void *pdsa_thread_init (void *ctxt)
{
    // opting for graceful termination
    SDK_THREAD_DFRD_TERM_INIT(ctxt);

    if (pdsa_init() < 0) {
        SDK_ASSERT("pdsa_init failed!");
    }

    return NULL;
}

}
