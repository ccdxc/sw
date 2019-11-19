//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Main entry point for the Pensando Distributed Services Agent (PDSA)
//---------------------------------------------------------------

#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/metaswitch/stubs/pdsa_stubs_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_init.hpp"

int main(void)
{
    if (!pdsa_stub_mgmt_init()) {
        return 1; 
    }

    return 0;
}
