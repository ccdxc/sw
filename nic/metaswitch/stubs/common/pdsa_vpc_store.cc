//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDSA stub VRF/VPC store
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pdsa_vpc_store.hpp"
#include "nic/metaswitch/stubs/common/pdsa_cookie.hpp"
#include "nic/metaswitch/stubs/common/pdsa_error.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"

namespace pdsa_stub {
template<> sdk::lib::slab* slab_obj_t<vpc_obj_t>::slab_ = nullptr;

void
vpc_slab_init (slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id)
{
    slabs_[slab_id].
        reset(sdk::lib::slab::factory("PDSA-VPC", slab_id, 
                                      sizeof(vpc_obj_t), 
                                      8,               // PDS_MAX_VPC/8 
                                      true, true, true));
    if (unlikely (!slabs_[slab_id])) {
        throw Error("SLAB creation failed for VPC");
    }
    vpc_obj_t::set_slab(slabs_[slab_id].get());

}

void vpc_obj_t::update_store (state_t* state, bool op_delete)
{
    if (!op_delete) {
        state->vpc_store().add_upd(this->key(), this);
    } else { 
        state->vpc_store().erase(this->key());
    }
}

} // End namespace

