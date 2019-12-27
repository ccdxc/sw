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
#include "smsiincl.h"
#include "smsincl.h"
}

namespace pds_ms {

// Used in the HAL stubs to convert from MS IfIndex to PDS IfIndex    
uint32_t ms_to_pds_ifindex(uint32_t ms_ifindex);
uint32_t ms_to_pds_eth_ifindex(uint32_t ms_ifindex);

// Used when progamming Linux route to convert from MS IfIndex to Linux IfIndex
NBB_LONG ms_to_lnx_ifindex(NBB_LONG ms_ifindex, NBB_ULONG location);
NBB_LONG lnx_to_ms_ifindex(NBB_LONG lnx_ifindex, NBB_ULONG location);

static constexpr uint32_t k_ms_lif_if_base =
    (lim::IfIndexBase::SOFTWARE_IF_INDEX_BASE |
     (AMB_LIM_SOFTWIF_DUMMY << lim::IfIndexBase::SOFTWIF_BASE_BIT_SHIFT) |
     lim::IfIndexBase::LIM_ALLOCATED_INDEX_BASE);
static constexpr uint32_t k_ms_lif_if_mask = 0xFFFFF000;

// Used in the Mgmt Stubs to convert from PDS IfIndex to MS IfIndex
static inline uint32_t pds_to_ms_ifindex(uint32_t pds_ifindex, uint32_t if_type) {
    if (if_type == IF_TYPE_ETH) {
        // MSB is reserved for MS
        // Assumption -
        // Resetting Type and Slot bits in PDS IfIndex will reset MSB.
        // So the rest of the info is still there for us to reverse-generate the 
        // PDS IfIndex from the MS IfIndex.
        return pds_ifindex & ~((IF_TYPE_MASK << IF_TYPE_SHIFT) | 
                (ETH_IF_SLOT_MASK << ETH_IF_SLOT_SHIFT));
    } else if (if_type == IF_TYPE_LIF) {
        return (k_ms_lif_if_base + LIF_IFINDEX_TO_LIF_ID(pds_ifindex));
    }
    // Return the pds_ifindex if it is not Eth or Lif
    return pds_ifindex;
}
 
// Used in Mgmt stubs to convert BD ID to MS IRB IfIndex
static inline uint32_t bd_id_to_ms_ifindex (uint32_t bd_id) {
    return ((lim::IfIndexBase::IRB_IF_INDEX_BASE + bd_id +
            (AMB_LIM_BRIDGE_DOMAIN_EVPN << lim::IfIndexBase::IRB_BASE_BIT_SHIFT)) |
            lim::IfIndexBase::LIM_ALLOCATED_INDEX_BASE);
}

// Used in Mgmt stubs to convert Loopback interface ID to MS SW/Loopback IfIndex
static inline uint32_t loopback_to_ms_ifindex (uint32_t loopback_if_id) {
    return ((lim::IfIndexBase::SOFTWARE_IF_INDEX_BASE + loopback_if_id +
            (AMB_LIM_SOFTWIF_LOOPBACK << lim::IfIndexBase::SOFTWIF_BASE_BIT_SHIFT)) |
            lim::IfIndexBase::LIM_ALLOCATED_INDEX_BASE);
}

// Used at MS initialization to set up SMI HW Desc from the PDS catalog
uint32_t pds_port_to_ms_ifindex_and_ifname(uint32_t port, std::string* ifname);

static inline uint32_t ms_ifindex_to_pds_type (uint32_t ms_ifindex) {
    if ((ms_ifindex & k_ms_lif_if_mask) == k_ms_lif_if_base) {
        return IF_TYPE_LIF;
    }
    if (ms_ifindex >= lim::IfIndexBase::LIM_ALLOCATED_INDEX_BASE) {
        // LIM allocated internal MS interface
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
    return IF_TYPE_NONE;
}

} // End namespace

#endif
