//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS MS interface index conversion routines
//--------------------------------------------------------------

#ifndef __PDS_MS_IFINDEX_HPP__
#define __PDS_MS_IFINDEX_HPP__

#include "nic/sdk/include/sdk/if.hpp"
#include <nbase.h>
#include <lim_interface_index_base.hpp>
extern "C" {
#include <a0spec.h>
#include <o0mac.h>
#include <a0user.h>
}

namespace pds_ms {

// Used in the HAL stubs to convert from MS IfIndex to PDS IfIndex    
uint32_t ms_to_pds_ifindex(uint32_t ms_ifindex);
uint32_t ms_to_pds_eth_ifindex(uint32_t ms_ifindex);

// Used when progamming Linux route to convert from MS IfIndex to Linux IfIndex
NBB_LONG ms_to_lnx_ifindex(NBB_LONG ms_ifindex, NBB_ULONG location);
NBB_LONG lnx_to_ms_ifindex(NBB_LONG lnx_ifindex, NBB_ULONG location);

// Used in the Mgmt Stubs to convert from PDS IfIndex to MS IfIndex
static inline uint32_t pds_to_ms_ifindex(uint32_t pds_ifindex) {
    // MSB is reserved for MS
    // Assumption -
    // Resetting Type and Slot bits in PDS IfIndex will reset MSB.
    // So the rest of the info is still there for us to reverse-generate the 
    // PDS IfIndex from the MS IfIndex.
    return pds_ifindex & ~((IF_TYPE_MASK << IF_TYPE_SHIFT) | 
                           (ETH_IF_SLOT_MASK << ETH_IF_SLOT_SHIFT));
}
 
// Used at MS initialization to set up SMI HW Desc from the PDS catalog
uint32_t pds_port_to_ms_ifindex_and_ifname(uint32_t port, std::string* ifname);

static inline uint32_t ms_ifindex_to_pds_type (uint32_t ms_ifindex) {
    if (ms_ifindex >= lim::IfIndexBase::LIM_ALLOCATED_INDEX_BASE) {
        // LIM allocated internsl MS interface
        // eg - VXLAN tunnel, IRB etc
        return IF_TYPE_NONE;
    }
    // We only create UplinkL3 and LIF interfaces in Metaswitch
    // Assumptions: 
    // Eth Uplinks have parent port ID set in the 17th bit
    // And this 24-bit hardware index is passed directly to MS
    if (ms_ifindex >= (1 << ETH_IF_PARENT_PORT_SHIFT)) {
        return IF_TYPE_L3;
    } 
    //TODO: This assumes that LIFs are created as first-class interfaces
    // in MS. Needs to be changed if LIFS are created as software interfaces
    return IF_TYPE_LIF;
}

} // End namespace

#endif
