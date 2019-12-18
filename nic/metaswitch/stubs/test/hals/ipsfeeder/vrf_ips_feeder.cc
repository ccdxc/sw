//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/metaswitch/stubs/test/hals/ipsfeeder/vrf_ips_feeder.hpp"

namespace pdsa_test {

void load_vrf_test_input ()
{
    static vrf_ips_feeder_t g_vrf_ips_feeder;
    test_params()->test_input = &g_vrf_ips_feeder;
}

} // End namespace
