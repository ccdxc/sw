//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS-MS stub Remote VXLAN Tunnel Endpoint store
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pds_ms_tep_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_slab_object.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"

namespace pds_ms {

template<> sdk::lib::slab* slab_obj_t<tep_obj_t>::slab_ = nullptr;

void
tep_slab_init (slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id)
{
    slabs_[slab_id].
        reset(sdk::lib::slab::factory("PDS-MS-TEP", slab_id, 
                                      sizeof(tep_obj_t), 
                                      100,   // 1/10 of PDS_MAX_TEP
                                      true, true, true));
    if (unlikely (!slabs_[slab_id])) {
        throw Error("SLAB creation failed for TEP");
    }
    tep_obj_t::set_slab(slabs_[slab_id].get());
}

tep_obj_t::tep_obj_t(ip_addr_t tep_ip_, pds_nexthop_group_id_t hal_uecmp_idx_,
                     pds_tep_id_t hal_tep_idx_) 
        : prop_(tep_ip_, hal_uecmp_idx_, hal_tep_idx_) 
{
    hal_oecmp_idx_guard = std::make_shared<ecmp_idx_guard_t>();
    SDK_TRACE_DEBUG("Allocated HAL Overlay ECMP Index %d for TEP %s",
                    hal_oecmp_idx_guard->idx(), ipaddr2str(&tep_ip_));
}

void tep_obj_t::update_store (state_t* state, bool op_delete)
{
    if (!op_delete) {
        state->tep_store().add_upd(this->key(), this);
        SDK_TRACE_DEBUG ("Add TEP %s to store: hal_tep_idx_ %ld, "
                         "hal_uecmp_idx_ %ld, hal_oecmp_idx_ %ld", 
                         ipaddr2str(&prop_.tep_ip), prop_.hal_tep_idx, 
                         prop_.hal_uecmp_idx, hal_oecmp_idx_guard->idx());
    } else { 
        state->tep_store().erase(this->key());
        SDK_TRACE_DEBUG ("Delete TEP %s from store", 
                         ipaddr2str(&prop_.tep_ip));
    }
}

} // End namespace
