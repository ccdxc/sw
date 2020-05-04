//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Stub state implementation
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state_init.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
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
    ip_track_slab_init(slabs_, PDS_MS_IP_TRACK_SLAB_ID);

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
    ip_track_internal_idx_gen_ =
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

static indirect_ps_obj_t*
alloc_indirect_ps_ (state_t* state, ms_ps_id_t indirect_pathset)
{
    std::unique_ptr<indirect_ps_obj_t> indirect_ps_obj_uptr
        (new indirect_ps_obj_t());
    auto obj = indirect_ps_obj_uptr.get();
    state->indirect_ps_store().add_upd(indirect_pathset,
                                       std::move(indirect_ps_obj_uptr));
    return obj;
}

// Fetch the latest underlay direct pathset for the TEP
// from the indirect pathset associated to the TEP
// Update the TEP<->Indirect mapping store if the mapping doesn't
// exist yet
// This mapping will not be deleted even if the Tunnel create fails
// Entry will be erased when the Pathset is deleted from Metaswitch
ms_hw_tbl_id_t
state_lookup_indirect_ps_and_map_ip (state_t* state,
                                     ms_ps_id_t indirect_pathset,
                                     const ip_addr_t& destip,
                                     bool ms_evpn_tep_ip)
{
    auto indirect_ps_obj = state->indirect_ps_store().get(indirect_pathset);
    if (indirect_ps_obj == nullptr) {
        // If the indirect ps is not present it could mean that that
        // MS create pointed to a black-hole and we did not know
        // that it was an indirect pathset
        PDS_TRACE_DEBUG("Set new indirect pathset %d -> blackhole",
                        indirect_pathset);
        indirect_ps_obj = alloc_indirect_ps_(state, indirect_pathset);

    } else if (!ip_addr_is_zero(&(indirect_ps_obj->destip()))) {

        // Assert there is only 1 IP referring to each indirect Pathset
        if (!ip_addr_is_equal (&(indirect_ps_obj->destip()), &destip)) {
            PDS_TRACE_ERR("Attempt to stitch %s %s to MS indirect pathset %d"
                          " that is already stitched to DestIP %s",
                          (ms_evpn_tep_ip) ? "TEP" : "tracked DestIP",
                          ipaddr2str(&destip), indirect_pathset,
                          ipaddr2str(&(indirect_ps_obj->destip())));
            SDK_ASSERT(0);
        }
        SDK_ASSERT(ms_evpn_tep_ip == indirect_ps_obj->is_ms_evpn_tep_ip());
        return indirect_ps_obj->direct_ps_dpcorr();
    }
    PDS_TRACE_DEBUG("Stitch %s %s to indirect pathset %d direct pathset %d",
                    (ms_evpn_tep_ip) ? "TEP" : "tracked DestIP",
                    ipaddr2str(&destip), indirect_pathset,
                    indirect_ps_obj->direct_ps_dpcorr());
    if (ms_evpn_tep_ip) {
        indirect_ps_obj->set_ms_evpn_tepip(destip);
    } else {
        indirect_ps_obj->set_destip(destip);
    }
    return indirect_ps_obj->direct_ps_dpcorr();
}

void
state_unmap_indirect_ps_from_ip (state_t* state, ms_ps_id_t indirect_pathset)
{
    auto indirect_ps_obj = state->indirect_ps_store().get(indirect_pathset);
    if (indirect_ps_obj == nullptr) {
        return;
    }
    PDS_TRACE_DEBUG("Unstitch %s %s from indirect pathset %d",
                    (indirect_ps_obj->is_ms_evpn_tep_ip()) ? "TEP" : "IP track",
                    ipaddr2str(&indirect_ps_obj->destip()), indirect_pathset);
    indirect_ps_obj->reset_destip();
}

// Update the indirect PS to direct PS mapping
// Return the Destination IP that is using this indirect PS if any
std::pair<ip_addr_t,bool>
state_indirect_ps_lookup_and_map_dpcorr (state_t* state,
                                         ms_ps_id_t indirect_pathset,
                                         ms_hw_tbl_id_t direct_ps_dpcorr)
{
    auto indirect_ps_obj = state->indirect_ps_store().get(indirect_pathset);
    if (indirect_ps_obj == nullptr) {
        PDS_TRACE_DEBUG("Set new indirect pathset %d -> direct pathset %d",
                        indirect_pathset, direct_ps_dpcorr);

        indirect_ps_obj = alloc_indirect_ps_(state, indirect_pathset);
        indirect_ps_obj->set_direct_ps_dpcorr(direct_ps_dpcorr); 

    } else if (indirect_ps_obj->direct_ps_dpcorr() != direct_ps_dpcorr) {
        PDS_TRACE_DEBUG("Update indirect pathset %d direct pathset %d -> %d",
                        indirect_pathset, indirect_ps_obj->direct_ps_dpcorr(),
                        direct_ps_dpcorr);
        indirect_ps_obj->set_direct_ps_dpcorr(direct_ps_dpcorr);
    }
    return std::pair<ip_addr_t,bool> (indirect_ps_obj->destip(),
                                      indirect_ps_obj->is_ms_evpn_tep_ip());
}


} // End namespace
