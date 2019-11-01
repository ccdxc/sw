//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Main entry point for the Pensando Distributed Services Agent (PDSA)
//---------------------------------------------------------------

#include "pdsa_state_init.hpp"
#include "pdsa_hal_init.hpp"
#include "pdsa_mgmt_init.hpp"

int main(void)
{
#ifdef NAPLES_BUILD
    if(!pdsa_stub::state_init()) {
        return -1;
    }
    if(!pdsa_stub::hal_init()) {
        goto error;
    }
#elif VENICE_BUILD
//  pdsa_stub_hal_dummy_init();
#endif
    if(!pdsa_stub_mgmt_init()) {
        goto error;
    }
    return 0;

error:
#ifdef NAPLES_BUILD
    pdsa_stub::state_deinit();
    pdsa_stub::hal_deinit();
#endif
    return -1;
}
