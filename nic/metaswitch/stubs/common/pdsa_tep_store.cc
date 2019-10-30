//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDSA stub Remote VXLAN Tunnel Endpoint store
//---------------------------------------------------------------

#include "pdsa_tep_store.hpp"

namespace pdsa_stub {

template<> sdk::lib::slab* slab_obj_t<tep_obj_t>::slab_ = nullptr;

const tep_dmac_t* tep_obj_t::dmac_info(const mac_addr_wr_t& mac) const
{
    auto it = dmacs_.find(mac);
    if (it == dmacs_.end()) {
        return nullptr;
    }
    return &(it->second);
}

bool tep_obj_t::dmac_info(const mac_addr_wr_t& mac, tep_dmac_t* out_dmac_entry) const
{
    auto it = dmacs_.find(mac);
    if (it == dmacs_.end()) {
        return false;
    }
    *out_dmac_entry = it->second;
    return true;
}

}
