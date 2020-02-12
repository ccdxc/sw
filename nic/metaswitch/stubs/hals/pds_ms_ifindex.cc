//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS MS interface index conversion routines
//--------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/sdk/lib/logger/logger.hpp"

namespace pds_ms {
using pds_ms::state_t;
using pds_ms::ms_ifindex_t;
using pds_ms::ms_iftype_t;
using pds_ms::if_obj_t;

uint32_t 
ms_to_pds_ifindex (uint32_t ms_ifindex)
{
    // Add back Type and Slot bits
    if (ms_ifindex_to_pds_type(ms_ifindex) == IF_TYPE_L3) {
        // Assumption: Uplinks have parent port ID set in the 17th bit
        ms_ifindex |= ((api::g_pds_state.catalogue()->slot() << ETH_IF_SLOT_SHIFT) 
                       & (ETH_IF_SLOT_MASK << ETH_IF_SLOT_SHIFT));
        return L3_IFINDEX(ms_ifindex);
    } else if (ms_ifindex_to_pds_type(ms_ifindex) == IF_TYPE_LIF) {
        auto lif_id = ms_ifindex - k_ms_lif_if_base;
        return LIF_IFINDEX(lif_id);
    }
    SDK_TRACE_ERR("If 0x%x: Unknown Metaswitch interface type", ms_ifindex);
    return ms_ifindex;
}

uint32_t 
ms_to_pds_eth_ifindex (uint32_t ms_ifindex)
{
    // Add back Type and Slot bits
    SDK_ASSERT (ms_ifindex_to_pds_type(ms_ifindex) == IF_TYPE_L3);
    // Assumption: Uplinks have parent port ID set in the 17th bit
    ms_ifindex |= ((api::g_pds_state.catalogue()->slot() << ETH_IF_SLOT_SHIFT) 
                   & (ETH_IF_SLOT_MASK << ETH_IF_SLOT_SHIFT));
    return ((IF_TYPE_ETH << IF_TYPE_SHIFT) | ms_ifindex);
}

std::string 
pds_ifindex_to_ifname (uint32_t pds_ifindex)
{
    // TODO : Get Linux IfName from LIF walk for EthIfIndex
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
    // ignore the conversion if it is ms generated ifindex
    if (ms_ifindex_to_pds_type (ms_ifindex) != IF_TYPE_L3) {
        return ms_ifindex;
    }
    auto state_ctxt = state_t::thread_context();
    auto phy_port_if_obj = state_ctxt.state()->if_store().get(ms_ifindex);
    SDK_ASSERT (phy_port_if_obj != nullptr);
    // Linux ifindex is cached in the store at the time 
    // of L3 interface creation
    auto lnx_ifindex = phy_port_if_obj->phy_port_properties().lnx_ifindex;
    SDK_TRACE_VERBOSE ("MS UserExit: MS IfIndex 0x%lx -> Linux IfIndex %ld", 
                       ms_ifindex, lnx_ifindex);
    return lnx_ifindex; 
}

NBB_LONG 
lnx_to_ms_ifindex (NBB_LONG lnx_ifindex, NBB_ULONG location)
{
    auto state_ctxt = state_t::thread_context();
    NBB_ULONG  ms_ifindex = 0;
    bool found = false;

    state_ctxt.state()->if_store().
        walk([lnx_ifindex, &ms_ifindex, &found] (ms_ifindex_t ifindex, if_obj_t& if_obj) ->bool {
            if (if_obj.type() == ms_iftype_t::VXLAN_PORT) {
                if (lnx_ifindex == ifindex) {
                    found = true;
                    ms_ifindex = ifindex;
                    return false;
                }
            }
            if (if_obj.type() != ms_iftype_t::PHYSICAL_PORT) {
                return true; // Continue walk
            }
            if (if_obj.phy_port_properties().lnx_ifindex == lnx_ifindex) {
                ms_ifindex = ifindex;
                found = true;
                return false; // Stop walk
            }
            return true;
        });

    if (found) {
        SDK_TRACE_VERBOSE("MS UserExit: Lnx IfIndex %ld -> MS IfIndex 0x%lx", 
                          lnx_ifindex, ms_ifindex);
    } else {
        SDK_TRACE_ERR("MS UserExit: Lnx IfIndex 0x%lx -> "
                      "Ms IfIndex conversion Failed", lnx_ifindex);
    }
    return ms_ifindex; 
}

// Used at MS initialization to set up SMI HW Desc from the PDS catalog
uint32_t pds_port_to_ms_ifindex_and_ifname (uint32_t port, std::string* ifname) 
{
    // Simulate an Ifindex that would come from NetAgent
    auto pds_ifindex = IFINDEX(IF_TYPE_UPLINK, 0, port, ETH_IF_DEFAULT_CHILD_PORT);
    *ifname = pds_ifindex_to_ifname (pds_ifindex);
    // Convert to MS IfIndex
    return pds_to_ms_ifindex(pds_ifindex, IF_TYPE_ETH);
}

} // End namespace
