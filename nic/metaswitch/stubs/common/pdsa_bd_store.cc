//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDSA stub Overlay Bridge Domain store
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pdsa_bd_store.hpp"
#include "nic/metaswitch/stubs/common/pdsa_cookie.hpp"
#include "nic/metaswitch/stubs/common/pdsa_error.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"

namespace pdsa_stub {
template<> sdk::lib::slab* slab_obj_t<bd_obj_t>::slab_ = nullptr;

void
bd_slab_init (slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id)
{
    slabs_[slab_id].
        reset(sdk::lib::slab::factory("PDSA-BD", slab_id, 
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

} // End namespace

