//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/metaswitch/stubs/test/hals/ipsfeeder/phy_port_ips_feeder.hpp"

namespace pdsa_test {

void load_phy_port_test_input ()
{
    static phy_port_ips_feeder_t g_phy_port_ips_feeder;
    test_params()->test_input = &g_phy_port_ips_feeder;
}

} // End namespace
