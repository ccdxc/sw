//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Stub state implementation
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state_init.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"
#include "nic/sdk/lib/logger/logger.hpp"

namespace pds_ms {

state_t* state_t::g_state_ = nullptr;
std::recursive_mutex state_t::g_mtx_;

template<> sdk::lib::slab* slab_obj_t<cookie_t>::slab_ = nullptr;

state_t::state_t(void)
{
    tep_slab_init(slabs_, PDS_MS_TEP_SLAB_ID);
    if_slab_init(slabs_, PDS_MS_IF_SLAB_ID);
    subnet_slab_init(slabs_, PDS_MS_SUBNET_SLAB_ID);
    bd_slab_init(slabs_, PDS_MS_BD_SLAB_ID);
    vpc_slab_init (slabs_, PDS_MS_VPC_SLAB_ID);
    mac_slab_init (slabs_, PDS_MS_MAC_SLAB_ID);
    route_table_slab_init (slabs_, PDS_MS_RTTABLE_SLAB_ID);
    pathset_slab_init (slabs_, PDS_MS_PATHSET_SLAB_ID);
    ecmp_idx_guard_slab_init (slabs_, PDS_MS_ECMP_IDX_GUARD_SLAB_ID);
    indirect_ps_slab_init(slabs_, PDS_MS_INDIRECT_PS_SLAB_ID);
    destip_track_slab_init(slabs_, PDS_MS_DESTIP_TRACK_SLAB_ID);

    slabs_[PDS_MS_COOKIE_SLAB_ID].
        reset(sdk::lib::slab::factory("PDS-MS-COOKIE",
                                      PDS_MS_COOKIE_SLAB_ID,
                                      sizeof(cookie_t),
                                      100,
                                      true, true, true));
    if (unlikely(!slabs_[PDS_MS_COOKIE_SLAB_ID])) {
        throw Error("SLAB creation failed for Cookie");
    }
    cookie_t::set_slab(slabs_[PDS_MS_COOKIE_SLAB_ID].get());

    // Index generator for PDS HAL Overlay ECMP table
    // Using 16 bit max index - 65535 Overlay ECMP Indexes
    ecmp_idx_gen_ = sdk::lib::rte_indexer::factory(0xFFFF-1,
                                                   /* skip zero */
                                                   true, true);
    // Index generator for internal IP used to create static routes
    // for tracking destination IP
    // Using 16 bit max index - 65535 internal IP
    destip_track_internal_idx_gen_ =
        sdk::lib::rte_indexer::factory(0xFFFF-1,
                                       /* skip zero */
                                       true, true);
}

bool
state_init (void)
{
    try {
        state_t::create();
    } catch (Error& e) {
        PDS_TRACE_ERR("Initialization failed - %s", e.what());
        return false;
    }
    PDS_TRACE_INFO ("State Initialization successful");
    return true;
}

void
state_destroy (void)
{
    state_t::destroy();
}

void state_store_commit_objs (state_t::context_t& state_ctxt,
                              std::vector<base_obj_uptr_t>& objs)
{
    if (objs.size() > 0) {PDS_TRACE_DEBUG ("Committing object(s) to store:");}

    for (auto& obj_uptr: objs) {
        obj_uptr->print_debug_str();
        obj_uptr->update_store (state_ctxt.state(), false);
        // New created object is saved in store.
        // Release the obj ownership from cookie
        obj_uptr.release();
    }
    objs.clear();
}
} // End namespace
