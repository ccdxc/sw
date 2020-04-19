//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS-MS stub Interface Store
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pds_ms_if_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"

namespace pds_ms {

template<> sdk::lib::slab* slab_obj_t<if_obj_t>::slab_ = nullptr;

// Phy + Loopback = 3
// LIFS = 1/8th Subnets = 8
// IRBs = 1/8th Subnets = 8
// VXLAN Tunnels = 1/10 Teps = 100
// VXLAN Ports = 1/10 Teps * 5 VNIs per Tep = 500
constexpr int k_max_if = 600;

void
if_slab_init (slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id)
{
    slabs_[slab_id].
        reset(sdk::lib::slab::factory("PDS-MS-IF", slab_id, 
                                      sizeof(if_obj_t), 
                                      k_max_if,
                                      true, true, true));
    if (unlikely (!slabs_[slab_id])) {
        throw Error("SLAB creation failed for IF");
    }
    if_obj_t::set_slab(slabs_[slab_id].get());
}

void if_obj_t::update_store(state_t* state, bool op_delete) {
    if (!op_delete) {
        state->if_store().add_upd(this->key(), this);
    } else { 
        state->if_store().erase(this->key());
    }
}

void if_obj_t::print_debug_str(void) {
    switch(prop_.iftype_) {
    case ms_iftype_t::PHYSICAL_PORT:    
        PDS_TRACE_DEBUG ("  - MS Interface 0x%lx for PHY port", 
                         prop_.phy_port_.ifindex);
        break;    
    case ms_iftype_t::VXLAN_TUNNEL:
        PDS_TRACE_DEBUG ("  - MS Interface 0x%lx for VXLAN Tunnel", 
                         prop_.vxt_.ifindex);
        break;    
    case ms_iftype_t::VXLAN_PORT:
        PDS_TRACE_DEBUG ("  - MS Interface 0x%lx for L3 VXLAN Port", 
                         prop_.vxp_.ifindex);
        break;    
    case ms_iftype_t::IRB:    
        PDS_TRACE_DEBUG ("  - MS Interface 0x%lx for IRB", 
                         prop_.irb_.ifindex);
        break;    
    default:
        break;
    };
}

} // End namespace
