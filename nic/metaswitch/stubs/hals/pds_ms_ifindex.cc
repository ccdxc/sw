//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS MS interface index conversion routines
//--------------------------------------------------------------

#include "nic/metaswitch/stubs/hals/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include <net/if.h>
#include <ifaddrs.h>
#include <netpacket/packet.h>

namespace pdsa_stub {

uint32_t 
ms_to_pds_ifindex (uint32_t ms_ifindex)
{
    // Add back Type and Slot bits
    if (ms_ifindex > 0xFFFF) {
        // Assumption: Uplinks have parent port ID set in the 17th bit
        ms_ifindex |= ((api::g_pds_state.catalogue()->slot() << ETH_IF_SLOT_SHIFT) 
                       & (ETH_IF_SLOT_MASK << ETH_IF_SLOT_SHIFT));
        return L3_IFINDEX(ms_ifindex);
    } else {
        // Assumption: Everything else is a LIF
        return LIF_IFINDEX(ms_ifindex);
    }
}

static std::string 
pds_ifindex_to_ifname (uint32_t pds_ifindex)
{
    // Hardcoding the interface name to be same as Linux interface name.
    // uplink 1 - dsc0
    // uplink 2 - dsc1
    // TODO: This needs to be changed to use L3 interface name
    // And also if linux interface name for the uplink is not fixed then 
    // need a way to derive dynamically here or in the FT-Stub
#ifdef SIM
    std::string if_name = "eth";
#else
    std::string if_name = "dsc";
#endif
    // TODO: Ignoring Child ports - revisit later
    if_name += std::to_string(ETH_IFINDEX_TO_PARENT_PORT(pds_ifindex)-1);
    return if_name;
}

NBB_LONG 
ms_to_lnx_ifindex (NBB_LONG ms_ifindex, NBB_ULONG location)
{
    auto pds_ifindex = ms_to_pds_ifindex(ms_ifindex);
    if (IFINDEX_TO_IFTYPE(pds_ifindex) != IF_TYPE_L3) {
        // Linux conversion is only needed for L3 IfIndex
        SDK_TRACE_ERR ("Request to convert invalid Iftype to Linux IfIndex 0x%lx", pds_ifindex);
        return ms_ifindex;
    }

    // Check if the mapping can be found in the state cache
    {
        auto state_thr_ctxt = state_t::thread_context();
        auto lnx_ifindex = state_thr_ctxt.state()->lnx_ifindex(pds_ifindex);
        if (lnx_ifindex != 0) {return lnx_ifindex;}
    }

    // Fetch mapping based on Interface name
    std::string ifname = pds_ifindex_to_ifname (pds_ifindex);
    auto lnx_ifindex = if_nametoindex(ifname.c_str());
    SDK_TRACE_VERBOSE ("MS UserExit: Cache MS IfIndex = %ld -> Linux Ifname %s, Linux IfIndex = %ld", 
                       ms_ifindex, ifname.c_str(), lnx_ifindex);

    // Save the mapping in state cache
    if (lnx_ifindex != 0) {
        auto state_thr_ctxt = state_t::thread_context();
        state_thr_ctxt.state()->set_lnx_ifindex(pds_ifindex, lnx_ifindex);
        if (lnx_ifindex != 0) {return lnx_ifindex;}
    }
    return lnx_ifindex; 
}

// Used at MS initialization to set up SMI HW Desc from the PDS catalog
uint32_t pds_port_to_ms_ifindex_and_ifname (uint32_t port, std::string* ifname) 
{
    // Simulate an Ifindex that would come from NetAgent
    auto pds_ifindex = IFINDEX(IF_TYPE_UPLINK, 0, port, 0);
    *ifname = pds_ifindex_to_ifname (pds_ifindex);
    // Convert to MS IfIndex
    return pds_to_ms_ifindex(pds_ifindex);
}

} // End namespace
