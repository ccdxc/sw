//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Mock PDS MS interface index conversion routines
//--------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"

namespace pds_ms {

// Pegasus does not require any interfaces to be populated in Metaswitch.
// Define mock APIs to satisfy the build since Pegasus image 
// shares the pdsa_mgmt library with the Naples image.

NBB_LONG 
ms_to_lnx_ifindex (NBB_LONG ms_ifindex, NBB_ULONG location)
{
    return ms_ifindex;
}

NBB_LONG 
lnx_to_ms_ifindex (NBB_LONG lnx_ifindex, NBB_ULONG location)
{
    return lnx_ifindex;
}

uint32_t 
ms_to_l3_ifindex (uint32_t ms_ifindex) 
{
    return ms_ifindex;
}

uint32_t 
port_to_ms_ifindex (uint32_t port) 
{
    return port;
}

uint32_t
ms_to_pds_ifindex (uint32_t ms_ifindex)
{
    return ms_ifindex;
}

std::string 
pds_ifindex_to_ifname (uint32_t pds_ifindex)
{
    return "";
}
}
