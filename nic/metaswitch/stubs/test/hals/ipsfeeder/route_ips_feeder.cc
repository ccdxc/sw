//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/metaswitch/stubs/test/hals/ipsfeeder/route_ips_feeder.hpp"

namespace pds_ms_test {

void load_route_test_input ()
{
    static route_ips_feeder_t g_route_ips_feeder;
    test_params()->test_input = &g_route_ips_feeder;
}

} // End namespace
