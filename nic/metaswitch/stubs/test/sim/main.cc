//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This is the main entry point for the SIM that runs Metasswitch stack with the 
//  pds stub integration code and mocks the PDS HAL
//----------------------------------------------------------------------------

#include "nic/metaswitch/stubs/pdsa_stubs_init.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/metaswitch/stubs/test/hals/test_params.hpp"

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

namespace pdsa_test {
test_params_t* test_params() {    
    static test_params_t  g_test_params;
    return &g_test_params;
}
} // End namespace pdsa_test

int
main (int argc, char **argv)
{
    // Call the mock pds init
    pds_init(nullptr);
    // This will start nbase
    pdsa_stub::init();

}
