//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS-MS stub Overlay ECMP HAL Index helper
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pds_ms_ecmp_idx_guard.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"

namespace pds_ms {
template<> sdk::lib::slab* slab_obj_t<ecmp_idx_guard_t>::slab_ = nullptr;

void
ecmp_idx_guard_slab_init (slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id)
{
    slabs_[slab_id].
        reset(sdk::lib::slab::factory("PDS-MS-OVERLAY-ECMP-INDEX-GUARD",
                                      slab_id, sizeof(ecmp_idx_guard_t),
                                      128,
                                      true, true, true));
    if (unlikely (!slabs_[slab_id])) {
        throw Error("SLAB creation failed for Overlay ECMP HAL Index");
    }
    ecmp_idx_guard_t::set_slab(slabs_[slab_id].get());
}

ecmp_idx_guard_t::ecmp_idx_guard_t() {
    // Allocate index
    // Enter thread-safe context to access/modify global state
    auto state_ctxt = pds_ms::state_t::thread_context();
    auto rs = state_ctxt.state()->ecmp_idx_alloc(&ecmp_idx_);
    if (rs != SDK_RET_OK) {
        throw Error(std::string("Overlay ECMP HAL Index alloc failed for with err ")
                    .append(std::to_string(rs)));
    }
}

ecmp_idx_guard_t::~ecmp_idx_guard_t() {
    // Free index
    // Enter thread-safe context to access/modify global state
    auto state_ctxt = pds_ms::state_t::thread_context();
    auto rs = state_ctxt.state()->ecmp_idx_free(ecmp_idx_);
    if (rs != SDK_RET_OK) {
        SDK_TRACE_ERR("Overlay ECMP HAL Index %d free failed with err %d",
                      ecmp_idx_, rs);
        return;
    }
    SDK_TRACE_VERBOSE("Freed Overlay ECMP HAL Index %d", ecmp_idx_);
}

} // End namespace


