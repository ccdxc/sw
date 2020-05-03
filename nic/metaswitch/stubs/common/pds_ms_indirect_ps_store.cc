//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS-MS stub indirect pathset store
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pds_ms_indirect_ps_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"

namespace pds_ms {
template<> sdk::lib::slab* slab_obj_t<indirect_ps_obj_t>::slab_ = nullptr;

void
indirect_ps_slab_init (slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id)
{
    slabs_[slab_id].
        reset(sdk::lib::slab::factory("PDS-MS-INDIRECT-PS", slab_id, 
                                      sizeof(indirect_ps_obj_t), 
                                      100,   // 1/10 of PDS_MAX_TEP
                                      true, true, true));
    if (unlikely (!slabs_[slab_id])) {
        throw Error("SLAB creation failed for Indirect Pathset");
    }
    indirect_ps_obj_t::set_slab(slabs_[slab_id].get());
}

static void set_zero_ip(ip_addr_t& ip_addr) {
    ip_addr_t zero_ip = {0};
    ip_addr = zero_ip;
}

indirect_ps_obj_t::indirect_ps_obj_t() {
    set_zero_ip(destip_);
}

void indirect_ps_obj_t::reset_destip(void) {
    set_zero_ip(destip_);
}

} // End namespace
