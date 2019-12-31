//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS-MS stub Overlay Bridge Domain store
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pds_ms_bd_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"

namespace pds_ms {
template<> sdk::lib::slab* slab_obj_t<bd_obj_t>::slab_ = nullptr;

void
bd_slab_init (slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id)
{
    slabs_[slab_id].
        reset(sdk::lib::slab::factory("PDS-MS-BD", slab_id, 
                                      sizeof(bd_obj_t), 
                                      8,               // PDS_MAX_SUBNET/8 
                                      true, true, true));
    if (unlikely (!slabs_[slab_id])) {
        throw Error("SLAB creation failed for BD");
    }
    bd_obj_t::set_slab(slabs_[slab_id].get());

}

void bd_obj_t::update_store (state_t* state, bool op_delete)
{
    if (!op_delete) {
        state->bd_store().add_upd(this->key(), this);
    } else { 
        state->bd_store().erase(this->key());
    }
}

void bd_obj_t::walk_macs(std::function<bool(const mac_addr_t& mac)> fn) {
    mac_store_.walk([&fn] (const mac_obj_t::key_t& key, mac_obj_t&) {
        return fn(key.mac);
    });
}
} // End namespace

