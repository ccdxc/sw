//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS-MS stub Remote VXLAN Tunnel Endpoint synchronous update store
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pds_ms_tep_sync_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_slab_object.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"

// Holds Tunnel info that is updated synchronously without waiting for
// Async response from HAL
//
namespace pds_ms {

template<> sdk::lib::slab* slab_obj_t<tep_sync_obj_t>::slab_ = nullptr;

void
tep_sync_slab_init (slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id)
{
    slabs_[slab_id].
        reset(sdk::lib::slab::factory("PDS-MS-TEP-SYNC", slab_id, 
                                      sizeof(tep_sync_obj_t), 
                                      100,   // 1/10 of PDS_MAX_TEP
                                      true, true, true));
    if (unlikely (!slabs_[slab_id])) {
        throw Error("SLAB creation failed for TEP Sync");
    }
    tep_sync_obj_t::set_slab(slabs_[slab_id].get());
}

tep_sync_obj_t::tep_sync_obj_t(const ip_addr_t& tep_ip_,
                     ms_hw_tbl_id_t hal_uecmp_idx_) 
        : prop_(tep_ip_, hal_uecmp_idx_) 
{
}
} // End namespace

