//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/metaswitch/stubs/test/hals/ipsfeeder/bd_ips_feeder.hpp"

namespace pdsa_test {

void load_bd_test_input ()
{
    static bd_ips_feeder_t g_bd_ips_feeder;
    test_params()->test_input = &g_bd_ips_feeder;
}

} // End namespace
