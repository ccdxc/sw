//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Mock SMI HW Description
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/hals_mock/pds_ms_smi_hw_desc_mock.hpp"
#include <lipi.h>
#include <lim_smi_mac.hpp>

//-------------------------------------------------------------------------
// Shared function used by LIM and SMI for MAC handling.
// Used to initialize LIM hardware manager MAC address pool.
// LIM allocates MAC address from this pool for 
// System MAC, IRB interfaces and Loopback interfaces
// The system MAC will be used as Routers MAC in Type-5 routes generated
// unless it is overridden by setting evpnIpVrfRoutersMac when creating VRF
//-------------------------------------------------------------------------
NBB_ULONG get_initial_mac_address(NBB_BYTE (&initial_mac)[ATG_L2_MAC_ADDR_LEN],
                                  NBB_BOOL system_mac_requested,
                                  NBB_ULONG suggested_number)
{
    NBB_ULONG total_num_macs = 255;
    ATG_SET_ZERO_MAC(initial_mac);
    initial_mac[5] = 1;
    return total_num_macs;
}

namespace smi
{
HwDesc*
create_hw_desc(void)
{
    static pdsa_stub::mock_smi_hw_desc_t g_mock_smi_hw_desc_;
    return &g_mock_smi_hw_desc_;
}
} // End namespace

