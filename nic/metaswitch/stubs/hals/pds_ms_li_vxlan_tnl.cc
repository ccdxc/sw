//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// LI VXLAN Tunnel HAL integration
//---------------------------------------------------------------

#include <thread>
#include "nic/metaswitch/stubs/hals/pds_ms_li_vxlan_tnl.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_li_vxlan_port.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/apollo/learn/learn_api.hpp"
#include <li_fte.hpp>
#include <li_lipi_slave_join.hpp>
#include <li_vxlan.hpp>

extern NBB_ULONG li_proc_id;

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

// Fetch the latest underlay direct pathset for the TEP
// from the indirect pathset associated to the TEP
// Update the TEP<->Indirect mapping store if the mapping doesn't
// exist yet
// This mapping will not be deleted even if the Tunnel create fails
// Entry will be erased when the Pathset is deleted from Metaswitch
static ms_hw_tbl_id_t lookup_indirect_ps_and_map_tep_ip_(state_t* state,
                                                     ms_ps_id_t indirect_pathset,
                                                     const ip_addr_t& tep_ip) {

    auto indirect_ps_obj = state->indirect_ps_store().get(indirect_pathset);
    SDK_ASSERT (indirect_ps_obj != nullptr);

    if (!ip_addr_is_zero(&(indirect_ps_obj->tep_ip))) {
        // Assert there is only 1 TEP referring to each indirect Pathset
        if (!ip_addr_is_equal (&(indirect_ps_obj->tep_ip), &tep_ip)) {
            PDS_TRACE_ERR("Attempt to stitch TEP %s to MS indirect pathset %d"
                          " that is already stitched to TEP %s",
                          ipaddr2str(&tep_ip), indirect_pathset,
                          ipaddr2str(&(indirect_ps_obj->tep_ip)));
            SDK_ASSERT(0);
        }
        return indirect_ps_obj->direct_ps_dpcorr;
    }
    PDS_TRACE_DEBUG("Stitch TEP %s to indirect pathset %d direct pathset %d",
                    ipaddr2str(&tep_ip), indirect_pathset,
                    indirect_ps_obj->direct_ps_dpcorr);
    indirect_ps_obj->tep_ip = tep_ip;
    return indirect_ps_obj->direct_ps_dpcorr;
}

static void unmap_indirect_ps_2_tep_ip_(state_t* state,
                                        ms_ps_id_t indirect_pathset) {
    PDS_TRACE_DEBUG("Unstitch TEP from indirect pathset %d", indirect_pathset);
    auto indirect_ps_obj = state->indirect_ps_store().get(indirect_pathset);
    if (indirect_ps_obj == nullptr) {
        return;
    }
    ip_addr_t zero_ip = {0};
    indirect_ps_obj->tep_ip = zero_ip;
}

void li_vxlan_tnl::parse_ips_info_(ATG_LIPI_VXLAN_ADD_UPDATE* vxlan_tnl_add_upd_ips) {
    ips_info_.if_index = vxlan_tnl_add_upd_ips->id.if_index;
    ATG_INET_ADDRESS& ms_dest_ip = vxlan_tnl_add_upd_ips->vxlan_settings.dest_ip;
    ms_to_pds_ipaddr(ms_dest_ip, &ips_info_.tep_ip);
    ATG_INET_ADDRESS& ms_src_ip = vxlan_tnl_add_upd_ips->vxlan_settings.source_ip;
    ms_to_pds_ipaddr(ms_src_ip, &ips_info_.src_ip);
    NBB_CORR_GET_VALUE(ips_info_.ms_upathset_dpcorr,
                       vxlan_tnl_add_upd_ips->vxlan_settings.dp_pathset_correlator);
    ips_info_.tep_ip_str = ipaddr2str(&ips_info_.tep_ip);
    NBB_CORR_GET_VALUE(ips_info_.ms_upathset,
                       vxlan_tnl_add_upd_ips->vxlan_settings.pathset_id);
    ips_info_.vxlan_tnl_add_upd_ips = vxlan_tnl_add_upd_ips;
}

void li_vxlan_tnl::fetch_store_info_(pds_ms::state_t* state) {
    store_info_.tun_if_obj = state->if_store().get(ips_info_.if_index);

    if (op_delete_) {
        if (unlikely(store_info_.tun_if_obj == nullptr)) {
            throw Error("VXLAN Tunnel delete for unknown IfIndex " +
                         std::to_string(ips_info_.if_index));
        }
        auto& tun_prop = store_info_.tun_if_obj->vxlan_tunnel_properties();
        store_info_.tep_obj = state->tep_store().get(tun_prop.tep_ip);
        SDK_ASSERT (store_info_.tep_obj != nullptr);

    } else {
        if (store_info_.tep_obj == nullptr) {
            store_info_.tep_obj = state->tep_store().get(ips_info_.tep_ip);
        }
        // Fetch the latest HAL underlay NHgroup ID for the TEP
        // Also associate the indirect pathset to the TEP
        store_info_.ms_upathset_dpcorr =
            lookup_indirect_ps_and_map_tep_ip_(state,
                                               ips_info_.ms_upathset,
                                               ips_info_.tep_ip);
    }
}

pds_tep_spec_t li_vxlan_tnl::make_pds_tep_spec_(void) {
    pds_tep_spec_t spec = {0};
    auto& tep_prop = store_info_.tep_obj->properties();
    spec.key = make_pds_tep_key_();
    spec.remote_ip = tep_prop.tep_ip;
    spec.ip_addr = tep_prop.src_ip;

    // HAL underlay NHGroup UUID is derived from the underlay pathset DP Corr
    if (store_info_.ms_upathset_dpcorr == PDS_MS_ECMP_INVALID_INDEX) {
        spec.nh_type = PDS_NH_TYPE_BLACKHOLE;
    } else {
        spec.nh_type = PDS_NH_TYPE_UNDERLAY_ECMP;
        spec.nh_group = msidx2pdsobjkey(store_info_.ms_upathset_dpcorr, true);
    }

    spec.type = PDS_TEP_TYPE_WORKLOAD;
    spec.nat = false;
    return spec;
}

pds_nexthop_group_spec_t li_vxlan_tnl::make_pds_onhgroup_spec_(void) {
    pds_nexthop_group_spec_t spec = {0};
    auto& tep_prop = store_info_.tep_obj->properties();
    spec.key = make_pds_onhgroup_key_();
    spec.type = PDS_NHGROUP_TYPE_OVERLAY_ECMP;
    spec.num_nexthops = 1;
    spec.nexthops[0].type = PDS_NH_TYPE_OVERLAY;
    // Use the TEP MS IfIndex as the TEP Index
    spec.nexthops[0].tep = msidx2pdsobjkey(tep_prop.hal_tep_idx);
    return spec;
}

void li_vxlan_tnl::create_obj_(void) {
    SDK_ASSERT(store_info_.tun_if_obj == nullptr);

    // Create new If Object but do not save it in the Global State yet
    // This automatically allocates a HAL Overlay ECMP table index
    std::unique_ptr<if_obj_t> new_if_obj
        (new if_obj_t(if_obj_t::vxlan_tunnel_properties_t
                      {ips_info_.if_index,
                          ips_info_.tep_ip}));
    // Update the local store info context so that the make_pds_spec
    // refers to the latest fields
    store_info_.tun_if_obj = new_if_obj.get();
    // Cache the new object in the cookie to revisit asynchronously
    // when the PDS API response is received
    cookie_uptr_->objs.push_back(std::move (new_if_obj));

    // Create new Tep Object but do not save it in the Global State yet
    // Use the MS Tunnel IfIndex as the HAL index for TEP table
    // ECMP Table index is allocated in constructor for every new TEP object
    std::unique_ptr<tep_obj_t> new_tep_obj
        (new tep_obj_t(ips_info_.tep_ip, ips_info_.src_ip,
                       ips_info_.ms_upathset, ips_info_.if_index));
    // Update the local store info context so that the make_pds_spec
    // refers to the latest fields
    store_info_.tep_obj = new_tep_obj.get();

    // Temporarily cache the new object in the cookie
    // until we finish batch commit
    cookie_uptr_->objs.push_back(std::move(new_tep_obj));
}

pds_batch_ctxt_guard_t li_vxlan_tnl::make_batch_pds_spec_(state_t::context_t&
                                                          state_ctxt) {
    pds_batch_ctxt_guard_t bctxt_guard_;
    sdk_ret_t ret = SDK_RET_OK;

    SDK_ASSERT(cookie_uptr_); // Cookie should not be empty
    pds_batch_params_t bp {PDS_BATCH_PARAMS_EPOCH, PDS_BATCH_PARAMS_ASYNC,
                           pds_ms::hal_callback,
                           cookie_uptr_.get()};
    auto bctxt = pds_batch_start(&bp);

    if (unlikely (!bctxt)) {
        throw Error("PDS Batch Start failed for TEP "
                    + ips_info_.tep_ip_str);
    }
    bctxt_guard_.set (bctxt);

    if (op_delete_) { // Delete
        // First delete Overlay ECMP entry before TEP entry to ensure
        // Overlay ECMP does not point to deleted TEP in hardware.
        auto onhgroup_key = make_pds_onhgroup_key_();
        if (!PDS_MOCK_MODE()) {
            ret = pds_nexthop_group_delete(&onhgroup_key, bctxt);
        }
        if (unlikely (ret != SDK_RET_OK)) {
            throw Error(std::string("PDS ECMP Delete failed for TEP ")
                        .append(ipaddr2str(&store_info_.tep_obj->properties().tep_ip))
                        .append(" err=").append(std::to_string(ret)));
        }

        auto tep_key = make_pds_tep_key_();
        if (!PDS_MOCK_MODE()) {
            ret = pds_tep_delete(&tep_key, bctxt);
        }
        if (unlikely (ret != SDK_RET_OK)) {
            throw Error(std::string("PDS TEP Delete failed for TEP ")
                        .append(ipaddr2str(&store_info_.tep_obj->properties().tep_ip))
                        .append(" err=").append(std::to_string(ret)));
        }

    } else { // Add or update
        auto tep_spec = make_pds_tep_spec_();
        if (op_create_) {
            if (!PDS_MOCK_MODE()) {
                ret = pds_tep_create(&tep_spec, bctxt);
            }
        } else {
            if (!PDS_MOCK_MODE()) {
                ret = pds_tep_update(&tep_spec, bctxt);
            }

            // For TEP underlay update need to update all L3 VXLAN Ports
            auto l_tep_obj = store_info_.tep_obj;

            l_tep_obj->walk_l3_vxlan_ports(
                [&state_ctxt, l_tep_obj, bctxt] (ms_ifindex_t vxp_ifindex) -> bool {
                    auto vxp_if_obj = state_ctxt.state()->if_store().get(vxp_ifindex);
                    if (vxp_if_obj == nullptr) {
                        PDS_TRACE_ERR("Cannot find L3 VXLAN Port 0x%x while updating TEP",
                                      vxp_ifindex);
                        return false; // continue loop
                    }
                    auto& vxp_prop = vxp_if_obj->vxlan_port_properties();
                    PDS_TRACE_DEBUG("TEP %s Updating Underlay ECMP Index for MS"
                                    " L3 VXLAN Port 0x%x",
                                    ipaddr2str(&vxp_prop.tep_ip), vxp_ifindex);
                    li_vxlan_port vxp;
                    vxp.add_pds_tep_spec(bctxt, state_ctxt.state(),
                                         vxp_if_obj, l_tep_obj,
                                         false /* Op Update */);
                    return false; // continue loop
                });
        }
        if (unlikely (ret != SDK_RET_OK)) {
            throw Error(std::string("PDS TEP Create or Update failed for TEP ")
                        .append(ips_info_.tep_ip_str)
                        .append(" err=").append(std::to_string(ret)));
        }

        if (op_create_) {
            // Create Overlay ECMP entry for a new TEP
            // No change in Overlay ECMP for TEP update
            auto onhgroup_spec = make_pds_onhgroup_spec_();
            if (!PDS_MOCK_MODE()) {
                ret = pds_nexthop_group_create(&onhgroup_spec, bctxt);
            }
            if (unlikely (ret != SDK_RET_OK)) {
                throw Error(std::string("PDS ECMP Create failed for TEP ")
                            .append(ips_info_.tep_ip_str)
                            .append(" err=").append(std::to_string(ret)));
            }
        }
    }
    return bctxt_guard_;
}

void li_vxlan_tnl::populate_cookie_ips_response_() {
    auto l_ms_tnl_ifindex = ips_info_.if_index;
    auto l_tep_ip = ips_info_.tep_ip;
    auto l_vxlan_tnl_add_upd_ips = ips_info_.vxlan_tnl_add_upd_ips;
    auto l_op_create = op_create_;
    auto l_ms_upathset = ips_info_.ms_upathset;
    auto l_ms_upathset_dpcorr = store_info_.ms_upathset_dpcorr;

    cookie_uptr_->send_ips_reply =
        [l_ms_tnl_ifindex, l_tep_ip, l_vxlan_tnl_add_upd_ips,
         l_op_create, l_ms_upathset, l_ms_upathset_dpcorr]
        (bool pds_status, bool ips_mock) -> void {
            PDS_TRACE_DEBUG("++++ TEP %s MS-If 0x%x Underlay pathset %d"
                            " HAL NHgroup %d Async reply %s ++++",
                            ipaddr2str(&l_tep_ip), l_ms_tnl_ifindex,
                            l_ms_upathset, l_ms_upathset_dpcorr,
                            (pds_status) ? "Success": "Failure")
            if (!pds_status) {
                if (l_op_create) {
                    auto state_ctxt = pds_ms::state_t::thread_context();
                    PDS_TRACE_ERR("TEP %s create HAL Failure revert store",
                                  ipaddr2str(&l_tep_ip));
                    state_ctxt.state()->tep_store().erase(l_tep_ip);
                    state_ctxt.state()->if_store().erase(l_ms_tnl_ifindex);
                } else {
                    // Update op is not expected to fail in HAL
                    PDS_TRACE_ERR("TEP %s update HAL Failure ",
                                  ipaddr2str(&l_tep_ip));
                    SDK_ASSERT(0);
                }
            }
            // ----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            // ----------------------------------------------------------------
            if (unlikely(ips_mock)) return; // UT

            NBB_CREATE_THREAD_CONTEXT
            NBS_ENTER_SHARED_CONTEXT(li_proc_id);
            NBS_GET_SHARED_DATA();

            auto key = li::VxLan::get_key(*l_vxlan_tnl_add_upd_ips);
            auto& vxlan_store = li::Fte::get().get_lipi_join()->get_vxlan_store();
            auto it = vxlan_store.find(key);
            if (it == vxlan_store.end()) {
                // MS Stub Stateless mode
                auto send_response =
                    li::VxLan::set_ips_rc(&l_vxlan_tnl_add_upd_ips->ips_hdr,
                                          (pds_status) ? ATG_OK : ATG_UNSUCCESSFUL);
                SDK_ASSERT(send_response);
                PDS_TRACE_DEBUG("VXLAN Tunnel 0x%x send IPS response"
                                " stateless mode", key);
                li::Fte::get().get_lipi_join()->
                    send_ips_reply(&l_vxlan_tnl_add_upd_ips->ips_hdr);
            } else {
                // MS Stub Stateful mode
                PDS_TRACE_DEBUG("VXLAN Tunnel 0x%x send IPS response"
                                " stateless mode", key);
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
}

NBB_BYTE li_vxlan_tnl::handle_add_upd_(state_t::context_t&& state_ctxt_in,
                                       bool ext_trigger) {

    NBB_BYTE rc = ATG_OK;
    pds_batch_ctxt_guard_t pds_bctxt_guard;
    cookie_t* cookie = nullptr;

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt (std::move(state_ctxt_in));

        // Fetch store info
        // Also updates indirect pathset store if this is a new TEP
        // or a new underlay pathset for this TEP
        fetch_store_info_(state_ctxt.state());

        if (store_info_.tep_obj != nullptr) {
            // Update
            PDS_TRACE_INFO ("TEP %s Update IPS Underlay Pathset %d"
                            " Underlay DP Corr %d",
                            ips_info_.tep_ip_str.c_str(), ips_info_.ms_upathset,
                            store_info_.ms_upathset_dpcorr);

            // Disassociate TEP from old underlay indirect pathset if there
            // is a change
            auto old_ms_upathset = store_info_.tep_obj->properties().ms_upathset;
            if (old_ms_upathset != ips_info_.ms_upathset) {
                PDS_TRACE_DEBUG("TEP %s unmap from old indirect ps %d",
                                ips_info_.tep_ip_str.c_str(), old_ms_upathset);
                unmap_indirect_ps_2_tep_ip_(state_ctxt.state(), old_ms_upathset);

                store_info_.tep_obj->properties().ms_upathset
                    = ips_info_.ms_upathset;
            }
        } else {
            // Create
            if (ext_trigger) {
                // Cannot create TEP as part of the underlay update.
                // This is possible if we had received an asynchronous failure
                // from HAL for the TEP create which would have erased the
                // TEP object from store but retained the MS Indirect pathset
                // back reference to the TEP IP.
                // Assuming that if Tunnel failed there there will be no
                // L3 VXLAN ports (Type 5 TEPs) from Metaswitch
                PDS_TRACE_INFO("Ignore underlay path update for unknown TEP %s",
                               ips_info_.tep_ip_str.c_str());
                return rc;
            }
            PDS_TRACE_INFO ("TEP %s Create IPS underlay pathset %d"
                            " underlay DP Corr %d",
                            ips_info_.tep_ip_str.c_str(), ips_info_.ms_upathset,
                            store_info_.ms_upathset_dpcorr);
            op_create_ = true;
            create_obj_(); // Create TEP store obj
        }
        if (store_info_.ms_upathset_dpcorr != ips_info_.ms_upathset_dpcorr) {
            PDS_TRACE_DEBUG("Detected parallel update to TEP %s (DP Corr: %d)"
                            " and underlay pathset %d (DP Corr: %d)",
                            ips_info_.tep_ip_str.c_str(), ips_info_.ms_upathset_dpcorr,
                            ips_info_.ms_upathset, store_info_.ms_upathset_dpcorr);
        }

        if (!ext_trigger) {
            // If called from external triggers like Underlay ECMP update
            // then the cookie is already populated with the appropriate
            // IPS for response
            populate_cookie_ips_response_();
        }

        pds_bctxt_guard = make_batch_pds_spec_(state_ctxt);

        // If we have batched multiple IPS earlier flush it now
        // Cannot add a Tunnel create to an existing batch
        state_ctxt.state()->flush_outstanding_pds_batch();

        // All processing complete, only batch commit remains -
        // safe to release the cookie_uptr_ unique_ptr

        // Move any store objects out of the cookie for
        // commiting to the store
        auto store_objs = std::move(cookie_uptr_->objs);

        rc = ATG_ASYNC_COMPLETION;
        cookie = cookie_uptr_.release();

        auto ret = learn::api_batch_commit(pds_bctxt_guard.release());
        if (unlikely (ret != SDK_RET_OK)) {
            delete cookie;
            throw Error(std::string("Batch commit failed for Add-Update TEP ")
                        .append(ips_info_.tep_ip_str)
                        .append(" err=").append(std::to_string(ret)));
        }
        PDS_TRACE_DEBUG ("TEP %s MS-If 0x%x Underlay pathset %d HAL NHgroup %d"
                         " Add/Upd PDS Batch commit successful",
                         ips_info_.tep_ip_str.c_str(), ips_info_.if_index,
                         ips_info_.ms_upathset, store_info_.ms_upathset_dpcorr);

        state_store_commit_objs (state_ctxt, store_objs);

    } // End of state thread_context
      // Do Not access/modify global state after this

    if (PDS_MOCK_MODE()) {
        // Call the HAL callback in PDS mock mode
        std::thread cb(pds_ms::hal_callback, SDK_RET_OK, cookie);
        cb.detach();
    }
    return rc;
}

// Underlay Indirect Pathset for the tunnel has been updated with a new
// DP correlator (ECMP 1->2 or 2->1 case)
NBB_BYTE li_vxlan_tnl::handle_upathset_update(state_t::context_t&& state_ctxt,
                                              tep_obj_t* tep_obj,
                                              ms_hw_tbl_id_t ms_upathset_dpcorr,
                                              cookie_uptr_t&& cookie_uptr) {
    // Mock as if VXLAN Tunnel update IPS is received
    ips_info_.src_ip = tep_obj->properties().src_ip;
    ips_info_.tep_ip = tep_obj->properties().tep_ip;
    // MS LIM IfIndex for the VXLAN Tunnel is used as the HAL TEP Index
    ips_info_.if_index = tep_obj->properties().hal_tep_idx;
    store_info_.tep_obj = tep_obj;
    store_info_.ms_upathset_dpcorr = ms_upathset_dpcorr;
    ips_info_.ms_upathset_dpcorr = ms_upathset_dpcorr;
    ips_info_.ms_upathset = tep_obj->properties().ms_upathset;

    cookie_uptr_ = std::move(cookie_uptr);
    return handle_add_upd_(std::move(state_ctxt), true);
}

NBB_BYTE li_vxlan_tnl::handle_add_upd_ips(ATG_LIPI_VXLAN_ADD_UPDATE* vxlan_tnl_add_upd_ips) {

    parse_ips_info_(vxlan_tnl_add_upd_ips);
    // Alloc new cookie and cache IPS
    cookie_uptr_.reset (new cookie_t);

    {
        auto state_ctxt = state_t::thread_context();
        return handle_add_upd_(std::move(state_ctxt), false);
    }
}

void li_vxlan_tnl::handle_delete(NBB_ULONG tnl_ifindex) {
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    op_delete_ = true;

    // MS stub Integration APIs do not support Async callback for deletes.
    // However since we should not block the MS NBase main thread
    // the HAL processing is always asynchronous even for deletes.
    // Assuming that Deletes never fail the Store is also updated
    // in a synchronous fashion for deletes so that it is in sync
    // if there is a subsequent create from MS.

    ips_info_.if_index = tnl_ifindex;
    // Empty cookie to force async PDS.
    cookie_uptr_.reset (new cookie_t);
    ip_addr_t tep_ip;

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();

        fetch_store_info_(state_ctxt.state());
        if (unlikely (store_info_.tep_obj == nullptr &&
                      store_info_.tun_if_obj == nullptr)) {
            // No change
            PDS_TRACE_INFO ("TEP IfIndex 0x%x could not find TEP obj",
                            tnl_ifindex);
            return;
        }
        PDS_TRACE_INFO ("TEP %s: Delete IPS", ipaddr2str(&tep_ip));

        tep_ip = store_info_.tep_obj->properties().tep_ip;

        unmap_indirect_ps_2_tep_ip_(state_ctxt.state(),
                                    store_info_.tep_obj->properties().ms_upathset);

        pds_bctxt_guard = make_batch_pds_spec_ (state_ctxt);

        cookie_uptr_->send_ips_reply =
            [tep_ip] (bool pds_status, bool ips_mock) -> void {
                // ----------------------------------------------------------------
                // This block is executed asynchronously when PDS response is rcvd
                // ----------------------------------------------------------------
                PDS_TRACE_DEBUG("++++ TEP %s Delete: Async reply %s ++++",
                                ipaddr2str(&tep_ip), (pds_status)?"Success": "Failure");
                // Deletes are not expected to fail
                SDK_ASSERT (pds_status);
            };

        // If we have batched multiple IPS earlier flush it now
        // VXLAN Tunnel deletion cannot be appended to an existing batch
        state_ctxt.state()->flush_outstanding_pds_batch();

        // All processing complete, only batch commit remains -
        // safe to release the cookie_uptr_ unique_ptr
        auto cookie = cookie_uptr_.release();
        auto ret = learn::api_batch_commit(pds_bctxt_guard.release());
        if (unlikely (ret != SDK_RET_OK)) {
            delete cookie;
            throw Error(std::string("Batch commit failed for delete TEP ")
                        .append(ipaddr2str(&tep_ip))
                        .append(" err=").append(std::to_string(ret)));
        }
        PDS_TRACE_DEBUG ("TEP %s: Delete PDS Batch commit successful",
                         ipaddr2str(&tep_ip));

        state_ctxt.state()->tep_store().erase(tep_ip);
        state_ctxt.state()->if_store().erase(ips_info_.if_index);
    } // End of state thread_context
      // Do Not access/modify global state after this
}

} // End namespace
