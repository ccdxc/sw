//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/metaswitch/stubs/test/hals/ipsfeeder/vxlan_ips_feeder.hpp"

namespace pds_ms_test {

void load_vxlan_test_input ()
{
    static vxlan_ips_feeder_t g_vxlan_ips_feeder;
    test_params()->test_input = &g_vxlan_ips_feeder;
}

} // End namespace
