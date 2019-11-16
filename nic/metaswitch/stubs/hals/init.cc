//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Main entry point for the Pensando Distributed Services Agent (PDSA)
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pdsa_state_init.hpp"
#include "nic/metaswitch/stubs/hals/pdsa_hal_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_init.hpp"

namespace pdsa_stub{
int init ()
{
    if(!pdsa_stub::state_init()) {
        return -1;
    }
    if(!pdsa_stub::hal_init()) {
        goto error;
    }

    return 0;

error:
    pdsa_stub::state_destroy();
    pdsa_stub::hal_deinit();
    return -1;
}
}
