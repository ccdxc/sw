//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// LI VXLAN Tunnel HAL integration
//---------------------------------------------------------------

#include <thread>
#include "nic/metaswitch/stubs/hals/pds_ms_hals_ecmp.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_li_vxlan_port.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include <hals_c_includes.hpp>
#include <hals_nhpi_slave_join.hpp>
#include <hals_ecmp.hpp>

extern NBB_ULONG hals_proc_id;

//-------------------------------------------------------------------
//       MS obj                  PDS HAL Spec
// a) Pathset containing
//    L3 interfaces  -> Underlay ECMP NHs (referenced by TEP entries)
// b) VXLAN Tunnel   -> TEP entry -> Overlay ECMP entry (ref by Type2 MAC,IP)
// c) L2 VXLAN Port  -> Unused (since Type 2 VNI comes from Egress BD)
// d) L3 VXLAN Port  -> TEP,VNI entry (referenced by Type5 Overlay ECMP)
// e) Pathset containing
//    L3 VXLAN Ports -> Overlay ECMP entry (ref by Type5 Prefix routes)
//--------------------------------------------------------------------

namespace pds_ms {

#define NHPI_GET_FIRST_NH(ips, list_ptr) \
       (ATG_NHPI_APPENDED_NEXT_HOP*) \
            NTL_OFF_LIST_TRAVERSE((ips), (list_ptr), NULL)

#define NHPI_GET_NEXT_NH(ips, list_ptr, cur_obj) \
       (ATG_NHPI_APPENDED_NEXT_HOP*) \
            NTL_OFF_LIST_TRAVERSE((ips), (list_ptr), cur_obj)


bool hals_ecmp_t::parse_ips_info_(ATG_NHPI_ADD_UPDATE_ECMP* add_upd_ecmp_ips) {

    // Only first 32 bits of the correlator are used for Pathset ID
    // Ref: psm_fte.cpp -> psm::Fte::increment_corr
    SDK_ASSERT (add_upd_ecmp_ips->pathset_id.correlator2 == 0);
    NBB_CORR_GET_VALUE (ips_info_.pathset_id, add_upd_ecmp_ips->pathset_id);

    bool process = true;
    auto list_p = &add_upd_ecmp_ips->next_hop_objects;
    ATG_NHPI_APPENDED_NEXT_HOP appended_next_hop;

    for (auto next_hop = NHPI_GET_FIRST_NH(add_upd_ecmp_ips, list_p);
         next_hop != NULL;
         next_hop = NHPI_GET_NEXT_NH(add_upd_ecmp_ips, list_p, next_hop)) {

        if (next_hop->next_hop_properties.destination_type == 
            ATG_NHPI_NEXT_HOP_DEST_NH) {
                // Indirect NH Needed for VXLAN Tunnel - 
                // Copy to programmed to make MS think it is programmed in HW
                NBB_MEMSET(&appended_next_hop, 0, sizeof(ATG_NHPI_APPENDED_NEXT_HOP));

                appended_next_hop.total_length = sizeof(ATG_NHPI_APPENDED_NEXT_HOP);
                appended_next_hop.next_hop_properties = next_hop->next_hop_properties;
                NTL_OFF_LIST_APPEND(add_upd_ecmp_ips,
                                    &add_upd_ecmp_ips->programmed_next_hop_objects,
                                    appended_next_hop.total_length,
                                    (NBB_VOID *)&appended_next_hop,
                                    NULL);
                // Inherit the lower level DP correlator
                add_upd_ecmp_ips->dp_correlator =
                    next_hop->next_hop_properties.indirect_next_hop_properties.
                    lower_level_dp_correlator;
                uint32_t corr;
                NBB_CORR_GET_VALUE(corr, next_hop->next_hop_properties.
                                   indirect_next_hop_properties.
                                   lower_level_dp_correlator);
                SDK_TRACE_VERBOSE("Indirect NH Copy lower level DP Correlator %d",
                                  corr);
                // Skip processing Indirect NH
                process = false;
        }
        else if (next_hop->next_hop_properties.destination_type != 
            ATG_NHPI_NEXT_HOP_DEST_PORT) {
            // Ignore other types of next-hops
            SDK_TRACE_DEBUG("Ignoring non-direct nexthops");
            return false;
        }
        ATG_NHPI_NEIGHBOR_PROPERTIES& prop = 
            next_hop->next_hop_properties.direct_next_hop_properties.neighbor;
        ips_info_.nexthops.emplace_back(prop.neighbor_l3_if_index, 
                                        prop.neighbor_id.mac_address);
        if (ms_ifindex_to_pds_type(prop.neighbor_l3_if_index) == IF_TYPE_L3) {
            ips_info_.pds_nhgroup_type = PDS_NHGROUP_TYPE_UNDERLAY_ECMP;
        } else {
            ips_info_.pds_nhgroup_type = PDS_NHGROUP_TYPE_OVERLAY_ECMP;
        }
    }
    ips_info_.num_added_nh = 
        NTL_OFF_LIST_GET_LEN(add_upd_ecmp_ips,
                             &add_upd_ecmp_ips->added_next_hop_objects);
    ips_info_.num_deleted_nh = 
        NTL_OFF_LIST_GET_LEN(add_upd_ecmp_ips,
                             &add_upd_ecmp_ips->deleted_next_hop_objects);
    return process;
}

void hals_ecmp_t::fetch_store_info_(state_t* state) {
    // Only called for overlay
    store_info_.pathset_obj = state->pathset_store().get(ips_info_.pathset_id);
}

pds_nexthop_group_key_t hals_ecmp_t::make_pds_nhgroup_key_(void) {
    pds_nexthop_group_key_t key; 
    if (ips_info_.pds_nhgroup_type == PDS_NHGROUP_TYPE_UNDERLAY_ECMP) {
        key.id = ips_info_.pathset_id;
    } else {
        key.id = store_info_.pathset_obj->hal_oecmp_idx_guard->idx();
    }
    return key;
}

void hals_ecmp_t::make_pds_underlay_nhgroup_spec_
                                 (pds_nexthop_group_spec_t& nhgroup_spec) {
    nhgroup_spec.num_nexthops = ips_info_.nexthops.size();

    int i = 0, num_repeats = 0;
    if (op_create_) {
        num_repeats = 1;
    } else {
        // The only update allowed is when the number of nexthops in the Group
        // gets cut by half due to a link failure. 
        // In this case the remaining set of nexthops need to repeated twice
        SDK_TRACE_DEBUG("MS ECMP %ld Update - setting repeat to 2", 
                        ips_info_.pathset_id);
        num_repeats = 2;
    }

    for (int repeat = 0; repeat < num_repeats; ++repeat) {
        for (auto& nh: ips_info_.nexthops) {
            // Nexthop key is unused
            nhgroup_spec.nexthops[i].type = PDS_NH_TYPE_UNDERLAY;
            nhgroup_spec.nexthops[i].l3_if.id = 
                ms_to_pds_ifindex(nh.ms_ifindex);
            memcpy(nhgroup_spec.nexthops[i].underlay_mac, nh.mac_addr.m_mac,
                   ETH_ADDR_LEN);
            SDK_TRACE_DEBUG("MS ECMP %ld Add NH MSIfIndex 0x%lx PDSIfIndex 0x%lx MAC %s",
                            ips_info_.pathset_id, nh.ms_ifindex, 
                            nhgroup_spec.nexthops[i].l3_if.id,
                            macaddr2str(nh.mac_addr.m_mac));
            ++i;
        }
    }
}

void hals_ecmp_t::make_pds_overlay_nhgroup_spec_
                                 (pds_nexthop_group_spec_t& nhgroup_spec,
                                  state_t::context_t& state_ctxt) {
    nhgroup_spec.num_nexthops = ips_info_.nexthops.size();
    int i = 0;
    for (auto& nh: ips_info_.nexthops) {
        nhgroup_spec.nexthops[i].type = PDS_NH_TYPE_OVERLAY;
        nhgroup_spec.nexthops[i].key.id = 0; // Unused for NHs pointing to TEPs

        auto vxp_if_obj = state_ctxt.state()->if_store().get(nh.ms_ifindex);
        SDK_ASSERT(vxp_if_obj != nullptr);
        auto& vxp_prop = vxp_if_obj->vxlan_port_properties();
        auto tep_obj = state_ctxt.state()->tep_store()
                                .get(vxp_if_obj->vxlan_port_properties().tep_ip);
        if (memcmp(vxp_if_obj->vxlan_port_properties().dmaci, nh.mac_addr.m_mac,
                   ETH_ADDR_LEN) != 0) {
            // Change in L3 VXLAN Port DMAC
            // This is basically the Router MAC advertised in Type 5 routes
            // TODO: Assuming that the same {TEP, VNI} will not advertise
            // multiple Router MACs. Hence blindly overwriting existing MAC
            MAC_ADDR_COPY(vxp_if_obj->vxlan_port_properties().dmaci, nh.mac_addr.m_mac);
            li_vxlan_port vxp;
            vxp.add_pds_tep_spec(store_info_.bctxt, vxp_if_obj, tep_obj,
                                 false /* Op Update */);
            SDK_TRACE_DEBUG("Change DMAC for Type5 TEP %s VNI %d L3 VXLAN Port"
                            " 0x%x to %s",
                            ipaddr2str(&vxp_prop.tep_ip), vxp_prop.vni,
                            vxp_prop.ifindex, macaddr2str(nh.mac_addr.m_mac));
        }
        nhgroup_spec.nexthops[i].tep.id = tep_obj->properties().hal_tep_idx;
        SDK_TRACE_DEBUG("Add Type5 TEP %s VNI %d Idx %x to Overlay NHGroup %d",
                        ipaddr2str(&vxp_prop.tep_ip), vxp_prop.vni,
                        nhgroup_spec.nexthops[i].tep.id, nhgroup_spec.key.id);
        ++i;
    }
}

pds_nexthop_group_spec_t hals_ecmp_t::make_pds_nhgroup_spec_(state_t::context_t&
                                                             state_ctxt) {
    pds_nexthop_group_spec_t nhgroup_spec = {0};
    nhgroup_spec.key = make_pds_nhgroup_key_();
    nhgroup_spec.type = ips_info_.pds_nhgroup_type;
    if (ips_info_.pds_nhgroup_type == PDS_NHGROUP_TYPE_UNDERLAY_ECMP) {
        make_pds_underlay_nhgroup_spec_(nhgroup_spec);
    } else {
        make_pds_overlay_nhgroup_spec_(nhgroup_spec, state_ctxt);
    }
    return nhgroup_spec;
}

pds_batch_ctxt_guard_t hals_ecmp_t::make_batch_pds_spec_(state_t::context_t&
                                                         state_ctxt) {
    pds_batch_ctxt_guard_t bctxt_guard_;
    sdk_ret_t ret = SDK_RET_OK;
    SDK_ASSERT(cookie_uptr_); // Cookie should have been alloc before
    pds_batch_params_t bp { PDS_BATCH_PARAMS_EPOCH, PDS_BATCH_PARAMS_ASYNC,
                            pds_ms::hal_callback,
                            cookie_uptr_.get() };
    store_info_.bctxt = pds_batch_start(&bp);

    if (unlikely (!store_info_.bctxt)) {
        throw Error(std::string("PDS Batch Start failed for MS ECMP ")
                    .append(std::to_string(ips_info_.pathset_id)));
    }
    bctxt_guard_.set(store_info_.bctxt);

    if (op_delete_) { // Delete
        auto nhgroup_key = make_pds_nhgroup_key_();
        if (!PDS_MOCK_MODE()) {
            ret = pds_nexthop_group_delete(&nhgroup_key, store_info_.bctxt);
        }
        if (unlikely (ret != SDK_RET_OK)) {
            throw Error(std::string("Delete PDS Nexthop Group failed for MS ECMP ")
                        .append(std::to_string(ips_info_.pathset_id)));
        }

    } else { // Add or update
        auto nhgroup_spec = make_pds_nhgroup_spec_(state_ctxt);
        if (op_create_) {
            if (!PDS_MOCK_MODE()) {
                ret = pds_nexthop_group_create(&nhgroup_spec, store_info_.bctxt);
            }
            if (unlikely (ret != SDK_RET_OK)) {
                throw Error(std::string("Create PDS Nexthop Group failed for MS ECMP ")
                            .append(std::to_string(ips_info_.pathset_id)));
            }
        } else {
            if (!PDS_MOCK_MODE()) {
                ret = pds_nexthop_group_update(&nhgroup_spec, store_info_.bctxt);
            }
            if (unlikely (ret != SDK_RET_OK)) {
                throw Error(std::string("Update PDS Nexthop Group failed for MS ECMP ")
                            .append(std::to_string(ips_info_.pathset_id)));
            }
        }
    }
    return bctxt_guard_;
}

void hals_ecmp_t::handle_add_upd_ips(ATG_NHPI_ADD_UPDATE_ECMP* add_upd_ecmp_ips) {
    add_upd_ecmp_ips->return_code = ATG_OK;
    if (!parse_ips_info_(add_upd_ecmp_ips)) {
        // Nothing to do
        return;
    }

    if (ips_info_.pds_nhgroup_type == PDS_NHGROUP_TYPE_UNDERLAY_ECMP) {
        auto num_nexthops = ips_info_.nexthops.size();

        if ((ips_info_.num_added_nh == num_nexthops) &&
            (ips_info_.num_deleted_nh == 0)) {
            op_create_ = true;
            SDK_TRACE_DEBUG ("MS Underlay ECMP %ld: Create IPS Num nexthops %ld", 
                             ips_info_.pathset_id, num_nexthops);
        } else {
            // NH Group Update - optimization to quickly update in-place in case of
            // link failure. Adding new Nexthops to an existing Group is not 
            // supported by HAL.
            SDK_ASSERT(ips_info_.num_added_nh == 0);

            auto prev_num_nexthops = (num_nexthops + ips_info_.num_deleted_nh); 

            // Only removal of exactly half the nexthops is supported for an 
            // NH Group update
            if ((num_nexthops*2) != prev_num_nexthops) {
                // Ignore this optimized update - 
                // MS will anyway program a separate NH Group that does not have
                // the deleted nexthops when the routing protocol converges and
                // then re-program each TEP with the new ECMP group
                SDK_TRACE_ERR("MS Underlay ECMP %ld Update - Number of nexthops"
                              " %d needs to be half of previous number %d -"
                              " Ignore this update", ips_info_.pathset_id,
                              num_nexthops, prev_num_nexthops); 
                return;
            }
            SDK_TRACE_DEBUG ("MS Underlay ECMP %ld: Update IPS (optimization)",
                             ips_info_.pathset_id);
        }
    }

    // Alloc new cookie to capture async info
    cookie_uptr_.reset (new cookie_t);
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    pathset_obj_uptr_t pathset_obj_uptr;

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();

        if (ips_info_.pds_nhgroup_type == PDS_NHGROUP_TYPE_OVERLAY_ECMP) {
            fetch_store_info_(state_ctxt.state());
            if (likely(store_info_.pathset_obj == nullptr)) {
                op_create_ = true;
                pathset_obj_uptr.reset(new pathset_obj_t(ips_info_.pathset_id));
                store_info_.pathset_obj = pathset_obj_uptr.get();
                SDK_TRACE_DEBUG ("MS Overlay ECMP %ld: Create IPS Num nexthops %ld", 
                                 ips_info_.pathset_id, ips_info_.nexthops.size());
            } else {
                // TODO: Handle Overlay ECMP Update if needed after checking with MS
                SDK_TRACE_ERR ("MS Overlay ECMP %ld: Update IPS Num nexthops %ld"
                               " NOT SUPPORTED", 
                               ips_info_.pathset_id, ips_info_.nexthops.size());
                return;
            }
        }
        pds_bctxt_guard = make_batch_pds_spec_(state_ctxt); 
        // If we have batched multiple IPS earlier flush it now
        // Cannot defer Nexthop updates
        state_ctxt.state()->flush_outstanding_pds_batch();
    } // End of state thread_context
      // Do Not access/modify global state after this

    auto l_overlay = (ips_info_.pds_nhgroup_type == PDS_NHGROUP_TYPE_OVERLAY_ECMP);
    auto pathset_id = ips_info_.pathset_id;
    cookie_uptr_->send_ips_reply = 
        [add_upd_ecmp_ips, pathset_id, l_overlay] (bool pds_status,
                                                   bool ips_mock) -> void {
            //-----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            //-----------------------------------------------------------------
            if (unlikely(ips_mock)) return; // UT

            NBB_CREATE_THREAD_CONTEXT
            NBS_ENTER_SHARED_CONTEXT(hals_proc_id);
            NBS_GET_SHARED_DATA();

            auto key = hals::Ecmp::get_key(*add_upd_ecmp_ips);

            uint32_t dp_corr;
            if (pds_status) {
                if (!l_overlay) {
                    // HAL Key (DP correlator) is the MS pathset ID itself
                    dp_corr = pathset_id;
                } else {
                    // Enter thread-safe context to access/modify global state
                    auto state_ctxt = pds_ms::state_t::thread_context();
                    auto pathset_obj =
                        state_ctxt.state()->pathset_store().get(pathset_id);
                    dp_corr = pathset_obj->hal_oecmp_idx_guard->idx();
                }
            }
            NBB_CORR_PUT_VALUE(add_upd_ecmp_ips->dp_correlator, dp_corr);
           
            // Copy all nexthops to the programmed next_hop_objects
            auto list_p = &add_upd_ecmp_ips->next_hop_objects;
            ATG_NHPI_APPENDED_NEXT_HOP appended_next_hop;

            for (auto next_hop = NHPI_GET_FIRST_NH(add_upd_ecmp_ips, list_p);
                 next_hop != NULL;
                 next_hop = NHPI_GET_NEXT_NH(add_upd_ecmp_ips, list_p, next_hop)) {

                NBB_MEMSET(&appended_next_hop, 0, sizeof(ATG_NHPI_APPENDED_NEXT_HOP));

                appended_next_hop.total_length = sizeof(ATG_NHPI_APPENDED_NEXT_HOP);
                appended_next_hop.next_hop_properties = next_hop->next_hop_properties;

                NTL_OFF_LIST_APPEND(add_upd_ecmp_ips,
                                    &add_upd_ecmp_ips->programmed_next_hop_objects,
                                    appended_next_hop.total_length,
                                    (NBB_VOID *)&appended_next_hop,
                                    NULL);
            }

            auto& ecmp_store = hals::Fte::get().get_nhpi_join()->get_ecmp_store();
            auto it = ecmp_store.find(key);
            if (it == ecmp_store.end()) {
                auto send_response = 
                    hals::Ecmp::set_ips_rc(&add_upd_ecmp_ips->ips_hdr,
                                         (pds_status)?ATG_OK:ATG_UNSUCCESSFUL);
                SDK_ASSERT(send_response);
                SDK_TRACE_DEBUG("Send ECMP %ld Async IPS response stateless mode",
                                pathset_id);
                hals::Fte::get().get_nhpi_join()->
                    send_ips_reply(&add_upd_ecmp_ips->ips_hdr);
            } else {
                SDK_TRACE_DEBUG("Send ECMP %ld Async IPS response stateful mode",
                                pathset_id);
                if (pds_status) {
                    (*it)->update_complete(ATG_OK);
                } else {
                    (*it)->update_failed(ATG_UNSUCCESSFUL);
                }
            }
            NBS_RELEASE_SHARED_DATA();
            NBS_EXIT_SHARED_CONTEXT();
            NBB_DESTROY_THREAD_CONTEXT    
        };

    // All processing complete, only batch commit remains - 
    // safe to release the cookie unique_ptr
    auto cookie = cookie_uptr_.release();
    auto ret = pds_batch_commit(pds_bctxt_guard.release());
    if (unlikely (ret != SDK_RET_OK)) {
        delete cookie;
        throw Error(std::string("Batch commit failed for Add-Update Nexthop ")
                    .append(std::to_string(ips_info_.pathset_id))
                    .append(" err=").append(std::to_string(ret)));
    }
    add_upd_ecmp_ips->return_code = ATG_ASYNC_COMPLETION;
    SDK_TRACE_DEBUG ("MS ECMP %ld: Add/Upd PDS Batch commit successful", 
                     ips_info_.pathset_id);
    if (PDS_MOCK_MODE()) {
        // Call the HAL callback in PDS mock mode
        std::thread cb(pds_ms::hal_callback, SDK_RET_OK, cookie);
        cb.detach();
    }
}

void hals_ecmp_t::handle_delete(NBB_CORRELATOR ms_pathset_id) {
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    op_delete_ = true;

    // MS Stub Integration APIs do not support Async callback for deletes.
    // However since we should not block the MS NBase main thread
    // the HAL processing is always asynchronous even for deletes. 
    // Assuming that Deletes never fail.

    NBB_CORR_GET_VALUE (ips_info_.pathset_id, ms_pathset_id);

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();
        // If we have batched multiple IPS earlier flush it now
        // Cannot defer Nexthop updates
        state_ctxt.state()->flush_outstanding_pds_batch();

        store_info_.pathset_obj = state_ctxt.state()->pathset_store().
                                            get(ips_info_.pathset_id);
        if (store_info_.pathset_obj != nullptr) {
            ips_info_.pds_nhgroup_type = PDS_NHGROUP_TYPE_OVERLAY_ECMP;
            SDK_TRACE_DEBUG ("MS Overlay ECMP %ld: Delete IPS",
                             ips_info_.pathset_id);
        } else {
            ips_info_.pds_nhgroup_type = PDS_NHGROUP_TYPE_UNDERLAY_ECMP;
            SDK_TRACE_DEBUG ("MS Underlay ECMP %ld: Delete IPS",
                             ips_info_.pathset_id);
        }
        // Empty cookie to force async PDS.
        cookie_uptr_.reset (new cookie_t);
        pds_bctxt_guard = make_batch_pds_spec_ (state_ctxt); 

    } // End of state thread_context
      // Do Not access/modify global state after this

    auto pathset_id = ips_info_.pathset_id;
    cookie_uptr_->send_ips_reply = 
        [pathset_id] (bool pds_status, bool ips_mock) -> void {
            //-----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            //-----------------------------------------------------------------
            SDK_TRACE_DEBUG("Async PDS ECMP %ld delete successful",
                            pathset_id);

        };

    // All processing complete, only batch commit remains - 
    // safe to release the cookie_uptr_ unique_ptr
    auto cookie = cookie_uptr_.release();
    auto ret = pds_batch_commit(pds_bctxt_guard.release());
    if (unlikely (ret != SDK_RET_OK)) {
        delete cookie;
        throw Error(std::string("Batch commit failed for delete MS ECMP ")
                    .append(std::to_string(ips_info_.pathset_id))
                    .append(" err=").append(std::to_string(ret)));
    }
    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();
        state_ctxt.state()->pathset_store().erase(ips_info_.pathset_id);
    }
    SDK_TRACE_DEBUG ("MS ECMP %ld: Delete PDS Batch commit successful", 
                     ips_info_.pathset_id);
}

} // End namespace
