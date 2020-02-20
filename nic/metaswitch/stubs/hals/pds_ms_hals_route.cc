//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// LI VXLAN Tunnel HAL integration
//---------------------------------------------------------------

#include <thread>
#include "nic/metaswitch/stubs/hals/pds_ms_hals_route.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hals_utils.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include <hals_c_includes.hpp>
#include <hals_ropi_slave_join.hpp>
#include <hals_route.hpp>

extern NBB_ULONG hals_proc_id;

namespace pds_ms {

using pds_ms::Error;

void hals_route_t::populate_route_id(ATG_ROPI_ROUTE_ID* route_id) {
    // Populate prefix
    memset(&ips_info_.pfx, 0, sizeof(ip_prefix_t));
    ATG_INET_ADDRESS& ms_addr = route_id->destination_address;
    ms_to_pds_ipaddr(ms_addr, &ips_info_.pfx.addr);
    ips_info_.pfx.len = route_id->prefix_length;
    // Populate VRF
    ips_info_.vrf_id = vrfname_2_vrfid(route_id->vrf_name,
                                       route_id->vrf_name_len);
}

bool hals_route_t::parse_ips_info_(ATG_ROPI_UPDATE_ROUTE* add_upd_route_ips) {
    populate_route_id(&add_upd_route_ips->route_id);
    // Populate the correlator
    NBB_CORR_GET_VALUE(ips_info_.overlay_ecmp_id, add_upd_route_ips->
                       route_properties.dp_pathset_correlator);
    return true;
}

pds_obj_key_t hals_route_t::make_pds_rttable_key_(void) {
    auto state = pds_ms::state_t::thread_context().state();
    auto vpc_obj = state->vpc_store().get(ips_info_.vrf_id);
    if (unlikely(vpc_obj == nullptr)) {
        throw Error("Cannot find VPC store obj for id " + ips_info_.vrf_id);
    }
    rttbl_key_ = vpc_obj->properties().vpc_spec.v4_route_table;
    return (rttbl_key_);
}

void hals_route_t::make_pds_rttable_spec_(pds_route_table_spec_t &rttable,
                                          const pds_obj_key_t& rttable_key) {
    memset(&rttable, 0, sizeof(pds_route_table_spec_t));
    rttable.key = rttable_key;
    rttable.af = IP_AF_IPV4;
    
    // Populate the new route
    route_.prefix = ips_info_.pfx;
    route_.nh_type = PDS_NH_TYPE_OVERLAY_ECMP;
    route_.nh_group = msidx2pdsobjkey(ips_info_.overlay_ecmp_id);
    { // Enter thread-safe context to access/modify global state
        auto state = pds_ms::state_t::thread_context().state();
        auto rttbl_store = state->route_table_store().get(rttable.key);
        if (unlikely(rttbl_store == nullptr)) {
            throw Error("Did not find route table store for VRF "
                                                    + ips_info_.vrf_id);
        }
        if (!op_delete_) {
            // Add/Update the new route in the store
            auto rt = rttbl_store->get_route(route_.prefix);
            if (rt == nullptr) {
                op_create_ = true;
            } else {
                // Cache the route for restore incase of failure
                prev_route_ = *rt;
            }
            rttbl_store->add_upd_route(route_);
        } else {
            // Delete the route from the store
            rttbl_store->del_route(route_.prefix);
        }
        // Get the current number of routes
        rttable.num_routes = rttbl_store->num_routes();
        // Get the routes pointer. PDS API will make a copy of the
        // route table and free it up once api processing is complete
        // after batch commit
        rttable.routes = rttbl_store->routes();
    }
    return;
}

pds_batch_ctxt_guard_t hals_route_t::make_batch_pds_spec_(const pds_obj_key_t&
                                                          rttable_key) {
    pds_batch_ctxt_guard_t bctxt_guard_;
    sdk_ret_t ret = SDK_RET_OK;

    SDK_ASSERT(cookie_uptr_); // Cookie should not be empty
    pds_batch_params_t bp {PDS_BATCH_PARAMS_EPOCH, PDS_BATCH_PARAMS_ASYNC,
                           pds_ms::hal_callback,
                           cookie_uptr_.get()};
    auto bctxt = pds_batch_start(&bp);
    if (unlikely(!bctxt)) {
        throw Error(std::string("PDS Batch Start failed for Route ")
                    .append(ippfx2str(&ips_info_.pfx)));
    }
    bctxt_guard_.set(bctxt);
    
    pds_route_table_spec_t rttbl_spec;
    // Delete is a route table update with the deleted route.
    // The route table is ONLY deleted when VRF gets deleted
    make_pds_rttable_spec_(rttbl_spec, rttable_key);
    if (!PDS_MOCK_MODE()) {
        ret = pds_route_table_update(&rttbl_spec, bctxt);
    }
    // Reset the route ptr to avoid free since the destructor
    // of pds_route_table_spec_t calls free
    rttbl_spec.routes = NULL;
    if (op_delete_) { // Delete
        if (unlikely (ret != SDK_RET_OK)) {
            throw Error(std::string("PDS Route Delete failed for pfx ")
                        .append(ippfx2str(&ips_info_.pfx))
                        .append(" err=").append(std::to_string(ret)));
        }
    } else { // Add or update
        if (unlikely (ret != SDK_RET_OK)) {
            throw Error(std::string("PDS Route Create or Update failed for pfx ")
                        .append(ippfx2str(&ips_info_.pfx))
                        .append(" err=").append(std::to_string(ret)));
        }
        // TODO: Is rollback for failure required here ?
    }
    return bctxt_guard_;
}

void hals_route_t::handle_add_upd_ips(ATG_ROPI_UPDATE_ROUTE* add_upd_route_ips) {
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    add_upd_route_ips->return_code = ATG_OK;

    parse_ips_info_(add_upd_route_ips);

    PDS_TRACE_DEBUG("Route Add IPS VRF %d Prefix %s Type %d Overlay ECMP %d",
                     ips_info_.vrf_id, ipaddr2str(&ips_info_.pfx.addr),
                     add_upd_route_ips->route_properties.type,
                     ips_info_.overlay_ecmp_id);

    if ((add_upd_route_ips->route_properties.type == ATG_ROPI_ROUTE_CONNECTED) ||
        (add_upd_route_ips->route_properties.type == ATG_ROPI_ROUTE_LOCAL_ADDRESS)) {
        PDS_TRACE_DEBUG("Ignore connected route");
        return;
    }

    // Alloc new cookie and cache IPS
    cookie_uptr_.reset (new cookie_t);

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();
        auto rttable_key = make_pds_rttable_key_();
        if (is_pds_obj_key_invalid(rttable_key)) {
            PDS_TRACE_DEBUG("Ignore MS route for VRF %d that does not"
                            " have Route table ID", ips_info_.vrf_id);
            return;
        }
        pds_bctxt_guard = make_batch_pds_spec_(rttable_key); 
        // Flush any outstanding batch
        state_ctxt.state()->flush_outstanding_pds_batch();
    } // End of state thread_context
      // Do Not access/modify global state after this
    
    auto l_prev_route = prev_route_;
    auto l_ips_info_ = ips_info_;
    auto l_rttbl_key_ = rttbl_key_;
    auto l_op_create_ = op_create_;
    cookie_uptr_->send_ips_reply = 
        [add_upd_route_ips, l_ips_info_, l_prev_route,
         l_op_create_, l_rttbl_key_]
        (bool pds_status, bool ips_mock) -> void {
            // ----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            // ----------------------------------------------------------------

            // Rollback the store incase of failure
            if (!pds_status) {
                auto state = pds_ms::state_t::thread_context().state();
                auto rttbl_store =
                    state->route_table_store().
                                get(l_rttbl_key_);
                if (l_op_create_) {
                    // Add failed. Delete the new route from the store
                    rttbl_store->del_route((ip_prefix_t &)l_ips_info_.pfx);
                } else {
                    // Update failed. Restore the old route
                    rttbl_store->add_upd_route((pds_route_t &)l_prev_route);
                }
            }
            if (unlikely(ips_mock)) return; // UT

            NBB_CREATE_THREAD_CONTEXT
            NBS_ENTER_SHARED_CONTEXT(hals_proc_id);
            NBS_GET_SHARED_DATA();

            do {
            auto ropi_join = get_hals_ropi_join();
            if (ropi_join == nullptr) {
                PDS_TRACE_ERR("Failed to find ROPI join to return AddUpd IPS");
                break;
            }
            auto& route_store = ropi_join->get_route_store();
            auto key = hals::Route::get_key(*add_upd_route_ips);
            auto it = route_store.find(key);
            if (it == route_store.end()) {
                auto send_response = 
                    hals::Route::set_ips_rc(&add_upd_route_ips->ips_hdr,
                                        (pds_status) ? ATG_OK : ATG_UNSUCCESSFUL);
                SDK_ASSERT(send_response);
                PDS_TRACE_DEBUG ("+++++++ MS Route %s: Send Async IPS "
                                 "reply %s stateless mode +++++++",
                                ippfx2str(&l_ips_info_.pfx),
                                (pds_status) ? "Success" : "Failure");
                ropi_join->send_ips_reply(&add_upd_route_ips->ips_hdr);
            } else {
                if (pds_status) {
                    PDS_TRACE_DEBUG("MS Route %s: Send Async IPS "
                                    "Reply success stateful mode",
                                    ippfx2str(&l_ips_info_.pfx));
                    (*it)->update_complete(ATG_OK);
                } else {
                    PDS_TRACE_DEBUG("MS Route %s: Send Async IPS "
                                    "Reply failure stateful mode",
                                    ippfx2str(&l_ips_info_.pfx));
                    (*it)->update_failed(ATG_UNSUCCESSFUL);
                }
            }
            } while (0);

            NBS_RELEASE_SHARED_DATA();
            NBS_EXIT_SHARED_CONTEXT();
            NBB_DESTROY_THREAD_CONTEXT
        };

    // All processing complete, only batch commit remains - 
    // safe to release the cookie_uptr_ unique_ptr
    auto cookie = cookie_uptr_.release();
    auto ret = pds_batch_commit(pds_bctxt_guard.release());
    if (unlikely (ret != SDK_RET_OK)) {
        delete cookie;
        throw Error(std::string("Batch commit failed for Add-Update Route ")
                    .append(ippfx2str(&ips_info_.pfx))
                    .append(" err=").append(std::to_string(ret)));
        //TODO: Is rollback required here ?
    }
    add_upd_route_ips->return_code = ATG_ASYNC_COMPLETION;
    PDS_TRACE_DEBUG ("Route %s: Add/Upd PDS Batch commit successful", 
                     ippfx2str(&ips_info_.pfx));
    if (PDS_MOCK_MODE()) {
        // Call the HAL callback in PDS mock mode
        std::thread cb(pds_ms::hal_callback, SDK_RET_OK, cookie);
        cb.detach();
    }
}

void hals_route_t::handle_delete(ATG_ROPI_ROUTE_ID route_id) {
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    op_delete_ = true;

    // MS stub Integration APIs do not support Async callback for deletes.
    // However since we should not block the MS NBase main thread
    // the HAL processing is always asynchronous even for deletes. 
    // Assuming that Deletes never fail the Store is also updated
    // in a synchronous fashion for deletes so that it is in sync
    // if there is a subsequent create from MS.

    populate_route_id(&route_id);
    PDS_TRACE_INFO ("MS Route %s: Delete IPS", ippfx2str(&ips_info_.pfx));

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();
        // Empty cookie
        cookie_uptr_.reset(new cookie_t);
        auto rttable_key = make_pds_rttable_key_();
        if (is_pds_obj_key_invalid(rttable_key)) {
            PDS_TRACE_DEBUG("Ignore MS route delete for VRF %d that does not"
                            " have Route table ID", ips_info_.vrf_id);
            return;
        }
        pds_bctxt_guard = make_batch_pds_spec_(rttable_key); 
        // If we have batched multiple IPS earlier flush it now
        // Cannot add Subnet Delete to an existing batch
        state_ctxt.state()->flush_outstanding_pds_batch();
    } // End of state thread_context
      // Do Not access/modify global state after this

    auto pfx = ips_info_.pfx;
    cookie_uptr_->send_ips_reply = 
        [pfx] (bool pds_status, bool ips_mock) -> void {
            // ----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            // ----------------------------------------------------------------
            PDS_TRACE_DEBUG("+++++ MS Route %s Delete: Rcvd Async PDS"
                            " response %s +++++++",
                         ippfx2str(&pfx), (pds_status) ? "Success" : "Failure");
        };
    // All processing complete, only batch commit remains - 
    // safe to release the cookie_uptr_ unique_ptr
    auto cookie = cookie_uptr_.release();
    auto ret = pds_batch_commit(pds_bctxt_guard.release());
    if (unlikely (ret != SDK_RET_OK)) {
        delete cookie;
        throw Error(std::string("Batch commit failed for delete MS Route ")
                    .append(ippfx2str(&pfx))
                    .append(" err=").append(std::to_string(ret)));
    }
    PDS_TRACE_DEBUG ("MS Route %s: Delete PDS Batch commit successful", 
                     ippfx2str(&pfx));
}

} // End namespace
