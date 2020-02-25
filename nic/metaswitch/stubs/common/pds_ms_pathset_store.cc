//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS-MS stub Overlay Pathset store for EVPN Type 5 routes
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pds_ms_pathset_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"

namespace pds_ms {
template<> sdk::lib::slab* slab_obj_t<pathset_obj_t>::slab_ = nullptr;

void
pathset_slab_init (slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id)
{
    slabs_[slab_id].
        reset(sdk::lib::slab::factory("PDS-MS-OVERLAY-PATHSET", slab_id, 
                                      sizeof(pathset_obj_t), 
                                      128,            
                                      true, true, true));
    if (unlikely (!slabs_[slab_id])) {
        throw Error("SLAB creation failed for Overlay Pathset");
    }
    pathset_obj_t::set_slab(slabs_[slab_id].get());

}

pathset_obj_t::pathset_obj_t(ms_ps_id_t ps_id) : prop_(ps_id) {
    // Allocate index
    hal_oecmp_idx_guard = make_shared<ecmp_idx_guard_t>();
    PDS_TRACE_DEBUG("Allocated HAL Overlay ECMP Index %d for MS Pathset %d",
                     hal_oecmp_idx_guard->idx(), prop_.ms_ps_id);
}

void pathset_obj_t::update_store (state_t* state, bool op_delete) {
    if (!op_delete) {
        state->pathset_store().add_upd(this->key(), this);
    } else { 
        state->pathset_store().erase(this->key());
    }
}

} // End namespace

