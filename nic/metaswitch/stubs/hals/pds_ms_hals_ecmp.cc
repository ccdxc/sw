//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// LI VXLAN Tunnel HAL integration
//---------------------------------------------------------------

#include <thread>
#include "nic/metaswitch/stubs/hals/pds_ms_hals_ecmp.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include <hals_c_includes.hpp>
#include <hals_nhpi_slave_join.hpp>
#include <hals_ecmp.hpp>
extern NBB_ULONG hals_proc_id;

namespace pds_ms {

using pds_ms::Error;

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

    auto list_p = &add_upd_ecmp_ips->next_hop_objects;
    for (auto next_hop = NHPI_GET_FIRST_NH(add_upd_ecmp_ips, list_p);
         next_hop != NULL;
         next_hop = NHPI_GET_NEXT_NH(add_upd_ecmp_ips, list_p, next_hop)) {

        if (next_hop->next_hop_properties.destination_type != 
            ATG_NHPI_NEXT_HOP_DEST_PORT) {
            // Ignore non direct next-hops
            SDK_TRACE_DEBUG("Ignoring non-direct nexthops");
            return false;
        }
        ATG_NHPI_NEIGHBOR_PROPERTIES& prop = 
            next_hop->next_hop_properties.direct_next_hop_properties.neighbor;
        ips_info_.nexthops.emplace_back(prop.neighbor_l3_if_index, 
                                        prop.neighbor_id.mac_address);
        if (ms_ifindex_to_pds_type(prop.neighbor_l3_if_index) == IF_TYPE_L3) {
            ips_info_.pds_nhgroup_type = PDS_NHGROUP_TYPE_UNDERLAY_ECMP;
        }
    }
    ips_info_.num_added_nh = 
        NTL_OFF_LIST_GET_LEN(add_upd_ecmp_ips,
                             &add_upd_ecmp_ips->added_next_hop_objects);
    ips_info_.num_deleted_nh = 
        NTL_OFF_LIST_GET_LEN(add_upd_ecmp_ips,
                             &add_upd_ecmp_ips->deleted_next_hop_objects);
    return true;
}

pds_nexthop_group_key_t hals_ecmp_t::make_pds_nhgroup_key_(void) {
    pds_nexthop_group_key_t key; 
    key.id = ips_info_.pathset_id;
    return key;
}

pds_nexthop_group_spec_t hals_ecmp_t::make_pds_nhgroup_spec_(void) {
    pds_nexthop_group_spec_t nhgroup_spec = {0};
    nhgroup_spec.key = make_pds_nhgroup_key_();
    nhgroup_spec.type = ips_info_.pds_nhgroup_type;
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
    return nhgroup_spec;
}

pds_batch_ctxt_guard_t hals_ecmp_t::make_batch_pds_spec_(void) {
    pds_batch_ctxt_guard_t bctxt_guard_;
    sdk_ret_t ret = SDK_RET_OK;
    SDK_ASSERT(cookie_uptr_); // Cookie should have been alloc before
    pds_batch_params_t bp { PDS_BATCH_PARAMS_EPOCH, PDS_BATCH_PARAMS_ASYNC,
                            pds_ms::hal_callback,
                            cookie_uptr_.get() };
    auto bctxt = pds_batch_start(&bp);

    if (unlikely (!bctxt)) {
        throw Error(std::string("PDS Batch Start failed for MS ECMP ")
                    .append(std::to_string(ips_info_.pathset_id)));
    }
    bctxt_guard_.set(bctxt);

    if (op_delete_) { // Delete
        auto nhgroup_key = make_pds_nhgroup_key_();
        if (!PDS_MOCK_MODE()) {
            ret = pds_nexthop_group_delete(&nhgroup_key, bctxt);
        }
        if (unlikely (ret != SDK_RET_OK)) {
            throw Error(std::string("Delete PDS Nexthop Group failed for MS ECMP ")
                        .append(std::to_string(ips_info_.pathset_id)));
        }

    } else { // Add or update
        auto nhgroup_spec = make_pds_nhgroup_spec_();
        if (op_create_) {
            if (!PDS_MOCK_MODE()) {
                ret = pds_nexthop_group_create(&nhgroup_spec, bctxt);
            }
            if (unlikely (ret != SDK_RET_OK)) {
                throw Error(std::string("Create PDS Nexthop Group failed for MS ECMP ")
                            .append(std::to_string(ips_info_.pathset_id)));
            }
        } else {
            if (!PDS_MOCK_MODE()) {
                ret = pds_nexthop_group_update(&nhgroup_spec, bctxt);
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

    if (ips_info_.pds_nhgroup_type != PDS_NHGROUP_TYPE_UNDERLAY_ECMP) {
        // Only underlay ECMP for now
        return;
    }
    auto num_nexthops = ips_info_.nexthops.size();

    if ((ips_info_.num_added_nh == num_nexthops) &&
        (ips_info_.num_deleted_nh == 0)) {
        op_create_ = true;
        SDK_TRACE_DEBUG ("MS ECMP %ld: Create IPS Num nexthops %ld", 
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
            SDK_TRACE_ERR("MS ECMP %ld Update - Number of nexthops %d needs to"
                          " be half of previous number %d - Ignore this update",
                         ips_info_.pathset_id, num_nexthops, prev_num_nexthops); 
            return;
        }
        SDK_TRACE_DEBUG ("MS ECMP %ld: Update IPS (optimization)", ips_info_.pathset_id);
    }

    // Alloc new cookie to capture async info
    cookie_uptr_.reset (new cookie_t);
    pds_batch_ctxt_guard_t  pds_bctxt_guard;

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();

        pds_bctxt_guard = make_batch_pds_spec_(); 
        // If we have batched multiple IPS earlier flush it now
        // Cannot defer Nexthop updates
        state_ctxt.state()->flush_outstanding_pds_batch();

    } // End of state thread_context
      // Do Not access/modify global state after this

    auto pathset_id = ips_info_.pathset_id;
    cookie_uptr_->send_ips_reply = 
        [add_upd_ecmp_ips, pathset_id] (bool pds_status, bool ips_mock) -> void {
            //-----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            //-----------------------------------------------------------------
            if (unlikely(ips_mock)) return; // UT

            NBB_CREATE_THREAD_CONTEXT
            NBS_ENTER_SHARED_CONTEXT(hals_proc_id);
            NBS_GET_SHARED_DATA();

            auto key = hals::Ecmp::get_key(*add_upd_ecmp_ips);
            // HAL Key (DP correlator) is the MS pathset ID itself
            if (pds_status) {
                NBB_CORR_PUT_VALUE(add_upd_ecmp_ips->dp_correlator, 
                                   pathset_id);
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
    SDK_TRACE_DEBUG ("MS ECMP %ld: Delete IPS", ips_info_.pathset_id);

    // Empty cookie to force async PDS.
    cookie_uptr_.reset (new cookie_t);
    pds_bctxt_guard = make_batch_pds_spec_ (); 

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();
        // If we have batched multiple IPS earlier flush it now
        // Cannot defer Nexthop updates
        state_ctxt.state()->flush_outstanding_pds_batch();
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
    SDK_TRACE_DEBUG ("MS ECMP %ld: Delete PDS Batch commit successful", 
                     ips_info_.pathset_id);
}

} // End namespace
