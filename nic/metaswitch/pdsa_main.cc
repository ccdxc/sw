// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Main entry point for the Pensando Distributed Services Agent (PDSA)

#include "pdsa_stub_hal_init.hpp"

int main ()
{
//    pdsa_stub_mgmt_init ();
//#ifdef NAPLES
    pdsa_stub_hal_init ();
//#else
//  pdsa_stub_hal_dummy_init();    
//#endif
    return 1;
}
