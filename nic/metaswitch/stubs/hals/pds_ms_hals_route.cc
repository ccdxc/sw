//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// LI VXLAN Tunnel HAL integration
//---------------------------------------------------------------

#include <thread>
#include "nic/metaswitch/stubs/hals/pds_ms_hals_route.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_ip_track_hal.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_tbl_idx.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hals_utils.hpp"
#include "nic/apollo/api/internal/pds_route.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/apollo/learn/learn_api.hpp"
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
    NBB_CORR_GET_VALUE(ips_info_.pathset_id, add_upd_route_ips->
                       route_properties.pathset_id);
    NBB_CORR_GET_VALUE(ips_info_.ecmp_id, add_upd_route_ips->
                       route_properties.dp_pathset_correlator);
    return true;
}

pds_obj_key_t hals_route_t::make_pds_rttable_key_(state_t* state) {
    auto vpc_obj = state->vpc_store().get(ips_info_.vrf_id);
    if (unlikely(vpc_obj == nullptr)) {
        throw Error("Cannot find VPC store obj for id " + ips_info_.vrf_id);
    }
    rttbl_key_ = vpc_obj->properties().vpc_spec.v4_route_table;
    return (rttbl_key_);
}

// Return true indicates route table has changed
bool hals_route_t::make_pds_rttable_spec_(state_t* state,
                                          pds_route_table_spec_t &rttable,
                                          const pds_obj_key_t& rttable_key) {
    memset(&rttable, 0, sizeof(pds_route_table_spec_t));
    rttable.key = rttable_key;
    bool ret = true;

    // Populate the new route
    route_.attrs.prefix = ips_info_.pfx;
    route_.attrs.nh_type = PDS_NH_TYPE_OVERLAY_ECMP;
    route_.attrs.nh_group = msidx2pdsobjkey(ips_info_.ecmp_id);

    auto rttbl_store = state->route_table_store().get(rttable.key);
    if (unlikely(rttbl_store == nullptr)) {
        throw Error("Did not find route table store for VRF "
                    + ips_info_.vrf_id);
    }
    if (!op_delete_) {
        // Add/Update the new route in the store
        auto rt = rttbl_store->get_route(route_.attrs.prefix);
        if (rt == nullptr) {
            op_create_ = true;
        } else {
            // Cache the route for restore incase of failure
            prev_route_ = *rt;
        }
        rttbl_store->add_upd_route(route_);

    } else {
        // Delete the route from the store
        ret = rttbl_store->del_route(route_.attrs.prefix);
    }
    // Get the routes pointer. PDS API will make a copy of the
    // route table and free it up once api processing is complete
    // after batch commit
    rttable.route_info = rttbl_store->routes();

    return ret;
}

pds_batch_ctxt_guard_t hals_route_t::make_batch_pds_spec_(state_t* state,
                                                          const pds_obj_key_t&
                                                          rttable_key) {
    pds_batch_ctxt_guard_t bctxt_guard_;
    sdk_ret_t ret = SDK_RET_OK;

    pds_route_table_spec_t rttbl_spec;
    // Delete is a route table update with the deleted route.
    // The route table is ONLY deleted when VRF gets deleted
    if (!make_pds_rttable_spec_(state, rttbl_spec, rttable_key)) {
        // Skip batch commit if there is no change
        // Return empty guard
        return  bctxt_guard_;
    }

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

    if (!PDS_MOCK_MODE()) {
        ret = pds_route_table_update(&rttbl_spec, bctxt);
    }

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

sdk_ret_t hals_route_t::underlay_route_add_upd_() {
    bool tracked = false;
    ip_addr_t destip;
    obj_id_t pds_obj_id;

    {
        auto state_ctxt = state_t::thread_context();
        auto state = state_ctxt.state();

        auto it_internal = state->ip_track_internalip_store().find(ips_info_.pfx);
        if (it_internal != state->ip_track_internalip_store().end()) {
            auto ip_track_obj = 
                state->ip_track_store().get(it_internal->second);
            destip = ip_track_obj->destip();
            pds_obj_id = ip_track_obj->pds_obj_id();
            tracked = true;

            // Add back ref from the indirect pathset to the route
            auto nhgroup_id = 
                state_lookup_indirect_ps_and_map_ip(state,ips_info_.pathset_id,
                                                    destip, false);
            ips_info_.ecmp_id = nhgroup_id;
        }
    }

    if (ips_info_.ecmp_id == PDS_MS_ECMP_INVALID_INDEX) {
        return SDK_RET_OK;
    }
    if (tracked) {
        return ip_track_reachability_change(destip, ips_info_.ecmp_id,
											pds_obj_id);
    }

    if (mgmt_state_t::thread_context().state()->overlay_routing_en()) {
        // No underlay route programming to HAL in overlay routing mode
        return SDK_RET_OK;
    }

    // TODO temporary until non-overlay TEP also starts using the new
    // DestIP track
    pds_route_spec_t route_spec = {0};
    auto& route_attrs = route_spec.attrs;
    route_attrs.prefix = ips_info_.pfx;
    route_attrs.nh_type = PDS_NH_TYPE_UNDERLAY_ECMP;
    route_attrs.nh_group = msidx2pdsobjkey(ips_info_.ecmp_id, true);
    return api::pds_underlay_route_update(&route_spec);
}

sdk_ret_t hals_route_t::underlay_route_del_() {
    // TODO: HAL objects associated with tracked Dest IPs need to be
    // black-holed when the underlay reachability is lost ??
    // We can also reach here when tracking is stopped in which case
    // the destip would have been deleted in which case there
    // is nothing to update - so need to differentiate these
    if (mgmt_state_t::thread_context().state()->overlay_routing_en()) {
        return SDK_RET_OK;
    }
    return api::pds_underlay_route_delete(&ips_info_.pfx);
}

void hals_route_t::overlay_route_del_() {
    op_delete_ = true;
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();
        // Empty cookie
        cookie_uptr_.reset(new cookie_t);
        auto rttable_key = make_pds_rttable_key_(state_ctxt.state());
        if (is_pds_obj_key_invalid(rttable_key)) {
            PDS_TRACE_DEBUG("Ignore MS route delete for VRF %d that does not"
                            " have Route table ID", ips_info_.vrf_id);
            return;
        }
        pds_bctxt_guard = make_batch_pds_spec_(state_ctxt.state(),
                                               rttable_key);
        if (!pds_bctxt_guard) {
            PDS_TRACE_DEBUG("Ignore %s route delete that is not in"
                            " route store", ippfx2str(&ips_info_.pfx));
            return;
        }
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
    auto ret = learn::api_batch_commit(pds_bctxt_guard.release());
    if (unlikely (ret != SDK_RET_OK)) {
        delete cookie;
        throw Error(std::string("Batch commit failed for delete MS Route ")
                    .append(ippfx2str(&pfx))
                    .append(" err=").append(std::to_string(ret)));
    }
    PDS_TRACE_DEBUG ("MS Route %s: Delete PDS Batch commit successful",
                     ippfx2str(&pfx));
}

NBB_BYTE hals_route_t::handle_add_upd_ips(ATG_ROPI_UPDATE_ROUTE* add_upd_route_ips) {
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    NBB_BYTE rc = ATG_OK;

    parse_ips_info_(add_upd_route_ips);

    if ((add_upd_route_ips->route_properties.type == ATG_ROPI_ROUTE_CONNECTED) ||
        (add_upd_route_ips->route_properties.type == ATG_ROPI_ROUTE_LOCAL_ADDRESS)) {
        PDS_TRACE_DEBUG("Ignore connected prefix %s route add",
                        ippfx2str(&ips_info_.pfx));
        // If overlay BGP peering is formed before subnet is configured locally
        // the GW IP address would have been advertised as Type 2 from remote DSC
        // and installed as a /32 route. Delete this route when it is identified
        // as a connected IP.
        overlay_route_del_();
        return rc;
    }

    PDS_TRACE_DEBUG("Route Add IPS VRF %d Prefix %s Type %d Pathset %d"
                    " Overlay NHgroup %d",
                     ips_info_.vrf_id, ippfx2str(&ips_info_.pfx),
                     add_upd_route_ips->route_properties.type,
                     ips_info_.pathset_id, ips_info_.ecmp_id);

    if (ips_info_.vrf_id == PDS_MS_DEFAULT_VRF_ID) {
        // We should not reach here in Overlay routing mode.
        // But Underlay only control-plane model requires user configured
        // VXLAN Tunnels to be stitched to underlay nexthop group.
        // Until the Metaswitch support for configured VXLAN tunnels
        // is available the stop-gap solution is to push underlay routes to
        // HAL API thread and have it stitch the TEPs to the Underlay NH groups.
        //
        // TODO: Special case - temporarily push underlay routes
        // to the internal API for walking and stitching TEPs.
        // This check to be removed later allowing it to fall through to
        // async batch mode when HAL API thread starts supporting
        // TEP stitching natively.
        return underlay_route_add_upd_();
    }

    if (ips_info_.ecmp_id == PDS_MS_ECMP_INVALID_INDEX) {
        // Can happen when the L2F UpdateRoutersMAC from EVPN is delayed
        // because of which PSM cannot fetch ARP MAC from NAR stub
        // and the ROPI route update comes with black-holed pathset
        PDS_TRACE_DEBUG("Ignore prefix route %s with black-holed pathset %d",
                        ippfx2str(&ips_info_.pfx), ips_info_.pathset_id);
        return rc;
    }

    // Alloc new cookie and cache IPS
    cookie_uptr_.reset (new cookie_t);

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();
        auto rttable_key = make_pds_rttable_key_(state_ctxt.state());
        if (is_pds_obj_key_invalid(rttable_key)) {
            PDS_TRACE_DEBUG("Ignore MS route for VRF %d that does not"
                            " have Route table ID", ips_info_.vrf_id);
            return rc;
        }
        pds_bctxt_guard = make_batch_pds_spec_(state_ctxt.state(),
                                               rttable_key);
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
                auto state_ctxt = pds_ms::state_t::thread_context();
                auto rttbl_store =
                    state_ctxt.state()->route_table_store().
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
                PDS_TRACE_DEBUG ("++++ MS Route %s Overlay NH Group %d Async "
                                 "reply %s stateless mode ++++",
                                ippfx2str(&l_ips_info_.pfx),
                                l_ips_info_.ecmp_id,
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
    rc = ATG_ASYNC_COMPLETION;
    auto cookie = cookie_uptr_.release();
    auto ret = learn::api_batch_commit(pds_bctxt_guard.release());
    if (unlikely (ret != SDK_RET_OK)) {
        delete cookie;
        throw Error(std::string("Batch commit failed for Add-Update Route ")
                    .append(ippfx2str(&ips_info_.pfx))
                    .append(" err=").append(std::to_string(ret)));
        //TODO: Is rollback required here ?
    }
    PDS_TRACE_DEBUG ("Route %s Overlay NHGroup %d Add/Upd PDS Batch"
                     " commit successful",
                     ippfx2str(&ips_info_.pfx), ips_info_.ecmp_id);
    if (PDS_MOCK_MODE()) {
        // Call the HAL callback in PDS mock mode
        std::thread cb(pds_ms::hal_callback, SDK_RET_OK, cookie);
        cb.detach();
    }
    return rc;
}

void hals_route_t::handle_delete(ATG_ROPI_ROUTE_ID route_id) {
    op_delete_ = true;

    // MS stub Integration APIs do not support Async callback for deletes.
    // However since we should not block the MS NBase main thread
    // the HAL processing is always asynchronous even for deletes.
    // Assuming that Deletes never fail the Store is also updated
    // in a synchronous fashion for deletes so that it is in sync
    // if there is a subsequent create from MS.

    populate_route_id(&route_id);
    {
        auto state_ctxt = pds_ms::state_t::thread_context();
        if (state_ctxt.state()->reset_ignored_prefix(ips_info_.pfx)) {
            PDS_TRACE_DEBUG("Ignore connected prefix %s route delete",
                            ippfx2str(&ips_info_.pfx));
            return;
        }
    }
    PDS_TRACE_INFO ("MS Route %s: Delete IPS", ippfx2str(&ips_info_.pfx));

    if (ips_info_.vrf_id == PDS_MS_DEFAULT_VRF_ID) {
        underlay_route_del_();
        return;
    }

    overlay_route_del_();
}

} // End namespace
