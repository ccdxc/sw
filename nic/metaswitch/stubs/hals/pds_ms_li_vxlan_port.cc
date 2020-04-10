//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// LI VXLAN Port HAL integration
//---------------------------------------------------------------

#include <thread>
#include "nic/metaswitch/stubs/hals/pds_ms_li_vxlan_port.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
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

void li_vxlan_port::parse_ips_info_(ATG_LIPI_VXLAN_PORT_ADD_UPD* vxlan_port_add_upd_ips) {
    ips_info_.if_index = vxlan_port_add_upd_ips->id.if_index;
    ATG_INET_ADDRESS& ms_dest_ip = vxlan_port_add_upd_ips->port_properties.remote_inet_addr;
    ms_to_pds_ipaddr(ms_dest_ip, &ips_info_.tep_ip);
    ATG_INET_ADDRESS& ms_src_ip = vxlan_port_add_upd_ips->port_properties.local_inet_addr;
    ms_to_pds_ipaddr(ms_src_ip, &ips_info_.src_ip);
    ips_info_.vni = vxlan_port_add_upd_ips->port_properties.remote_vni;
}

void li_vxlan_port::fetch_store_info_(pds_ms::state_t* state) {
    store_info_.vxp_if_obj = state->if_store().get(ips_info_.if_index);
    if (op_delete_) {
        if (store_info_.vxp_if_obj == nullptr) {
            return;
        }
        ips_info_.tep_ip = store_info_.vxp_if_obj->vxlan_port_properties().tep_ip;
    }
    store_info_.tep_obj = state->tep_store().get(ips_info_.tep_ip);
    if (unlikely(store_info_.tep_obj == nullptr)) {
        throw Error(std::string("Unknown VXLAN Tunnel ")
                    .append(ipaddr2str(&ips_info_.tep_ip)));
    }
}

pds_tep_spec_t li_vxlan_port::make_pds_tep_spec_(void) {
    pds_tep_spec_t spec = {0};
    auto& vxp_prop = store_info_.vxp_if_obj->vxlan_port_properties();
    spec.key = make_pds_tep_key_(vxp_prop);
    spec.remote_ip = vxp_prop.tep_ip;
    spec.type = PDS_TEP_TYPE_IGW;
    spec.encap.type = PDS_ENCAP_TYPE_VXLAN;
    spec.encap.val.vnid = vxp_prop.vni;
    MAC_ADDR_COPY(spec.mac, vxp_prop.dmaci);

    spec.ip_addr = ips_info_.src_ip;
    auto& tep_prop = store_info_.tep_obj->properties();
    if (tep_prop.hal_uecmp_idx == PDS_MS_ECMP_INVALID_INDEX) {
        // Metaswitch PSM deletes underlay Nexthops independent of EVPN.
        // So its possible that when a L3 VXLAN Port is being added the
        // underlying VXLAN tunnel has lost its underlay nexthop.
        // One example -
        // Special sequence of events caused by the static default route
        // that kicks in immediately if both BGP underlay session routes
        // are removed causing the deleted VXLAN ports to be added back
        // immediately even though the underlyng TEP may not have been
        // attached to underlay ECMP yet.
        PDS_TRACE_DEBUG("Set Blackhole Underlay NH Group for Type5 TEP");
        spec.nh_type = PDS_NH_TYPE_BLACKHOLE;
    } else {
        spec.nh_type = PDS_NH_TYPE_UNDERLAY_ECMP;
        // Underlay NH is shared with Type 2 TEP
        PDS_TRACE_DEBUG("Set Underlay NH Group %d for Type 5 TEP", tep_prop.hal_uecmp_idx);
        spec.nh_group = msidx2pdsobjkey(tep_prop.hal_uecmp_idx, true);
    }
    spec.nat = false;
    return spec;
}

void li_vxlan_port::add_pds_tep_spec(pds_batch_ctxt_t bctxt,
                                     if_obj_t* vxp_if_obj,
                                     tep_obj_t* tep_obj,
                                     bool op_create) {
    store_info_.vxp_if_obj = vxp_if_obj;
    store_info_.tep_obj = tep_obj;
    auto vxp_prop = vxp_if_obj->vxlan_port_properties();

    sdk_ret_t ret = SDK_RET_OK;
    auto tep_spec = make_pds_tep_spec_();
    if (op_create_) {
        if (!PDS_MOCK_MODE()) {
            ret = pds_tep_create(&tep_spec, bctxt);
        }
    } else {
        if (!PDS_MOCK_MODE()) {
            ret = pds_tep_update(&tep_spec, bctxt);
        }
    }
    if (unlikely (ret != SDK_RET_OK)) {
        throw Error(std::string("PDS TEP Create or Update failed for Type5 TEP ")
                    .append(ipaddr2str(&vxp_prop.tep_ip))
                    .append("VNI ").append(std::to_string(vxp_prop.vni))
                    .append(" err=").append(std::to_string(ret)));
    }
}

void li_vxlan_port::cache_obj_in_cookie_(void) {
    // Create new If Object but do not save it in the Global State yet
    std::unique_ptr<if_obj_t> new_if_obj
        (new if_obj_t(if_obj_t::vxlan_port_properties_t
                      {ips_info_.if_index,
                       ips_info_.tep_ip,
                       ips_info_.vni,
                       msidx2pdsobjkey(ips_info_.if_index) // HAL TEP Index
                      }));
    // Update the local store info context so that the make_pds_spec
    // refers to the latest fields
    store_info_.vxp_if_obj = new_if_obj.get();
    // Cache the new object in the cookie to revisit asynchronously
    // when the PDS API response is received
    cookie_uptr_->objs.push_back(std::move (new_if_obj));
}

pds_batch_ctxt_guard_t li_vxlan_port::make_batch_pds_spec_() {
    pds_batch_ctxt_guard_t bctxt_guard_;
    sdk_ret_t ret = SDK_RET_OK;

    SDK_ASSERT(cookie_uptr_); // Cookie should not be empty
    pds_batch_params_t bp {PDS_BATCH_PARAMS_EPOCH, PDS_BATCH_PARAMS_ASYNC,
                           pds_ms::hal_callback,
                           cookie_uptr_.get()};
    auto bctxt = pds_batch_start(&bp);

    if (unlikely (!bctxt)) {
        throw Error(std::string("PDS Batch Start failed for Type 5 TEP ")
                    .append(ipaddr2str(&ips_info_.tep_ip)));
    }
    bctxt_guard_.set (bctxt);

    auto& vxp_prop = store_info_.vxp_if_obj->vxlan_port_properties();

    if (op_delete_) { // Delete
        auto tep_key = make_pds_tep_key_(vxp_prop);
        if (!PDS_MOCK_MODE()) {
            ret = pds_tep_delete(&tep_key, bctxt);
        }
        if (unlikely (ret != SDK_RET_OK)) {
            throw Error(std::string("PDS TEP Delete failed for Type5 TEP ")
                        .append(ipaddr2str(&vxp_prop.tep_ip))
                        .append("VNI ").append(std::to_string(vxp_prop.vni))
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
        }
        if (unlikely (ret != SDK_RET_OK)) {
            throw Error(std::string("PDS TEP Create or Update failed for Type5 TEP ")
                        .append(ipaddr2str(&ips_info_.tep_ip))
                        .append("VNI ").append(std::to_string(vxp_prop.vni))
                        .append(" err=").append(std::to_string(ret)));
        }
    }
    return bctxt_guard_;
}

NBB_BYTE li_vxlan_port::handle_add_upd_ips(ATG_LIPI_VXLAN_PORT_ADD_UPD* vxlan_port_add_upd_ips) {

    NBB_BYTE rc = ATG_OK;

    parse_ips_info_(vxlan_port_add_upd_ips);
    if (!vxlan_port_add_upd_ips->port_properties.l3_capable) {
        // Only interested in L3 VXLAN Ports
            PDS_TRACE_INFO ("Ignore Type2 TEP %s Local VNI %d Remote VNI %d MS L2 VXLAN Port 0x%x IPS",
                            ipaddr2str(&ips_info_.tep_ip),
                            vxlan_port_add_upd_ips->port_properties.local_vni,
                            ips_info_.vni, ips_info_.if_index);
        return rc;
    }

    pds_batch_ctxt_guard_t  pds_bctxt_guard;

    // Alloc new cookie and cache IPS, VXLAN Port If Obj
    cookie_uptr_.reset (new cookie_t);

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();
        fetch_store_info_(state_ctxt.state());

        if (store_info_.vxp_if_obj != nullptr) {
            // Update Port
            PDS_TRACE_INFO ("Type5 TEP %s Local VNI %d Remote VNI %d MS L3 VXLAN Port 0x%x Update IPS",
                            ipaddr2str(&ips_info_.tep_ip),
                            vxlan_port_add_upd_ips->port_properties.local_vni,
                            ips_info_.vni, ips_info_.if_index);
        } else {
            // Create VXLAN Port
            PDS_TRACE_INFO ("Type5 TEP %s Local VNI %d Remote VNI %d MS L3 VXLAN Port 0x%x Create IPS",
                            ipaddr2str(&ips_info_.tep_ip),
                            vxlan_port_add_upd_ips->port_properties.local_vni,
                            ips_info_.vni, ips_info_.if_index);
            op_create_ = true;
        }
        cache_obj_in_cookie_();
        pds_bctxt_guard = make_batch_pds_spec_();

        // If we have batched multiple IPS earlier flush it now
        // Cannot add a VXLAN Port create to an existing batch
        state_ctxt.state()->flush_outstanding_pds_batch();

    } // End of state thread_context
      // Do Not access/modify global state after this

    auto l_tep_ip = ips_info_.tep_ip;
    auto l_vni = ips_info_.vni;
    cookie_uptr_->send_ips_reply =
        [l_tep_ip, l_vni, vxlan_port_add_upd_ips] (bool pds_status, bool ips_mock) -> void {
            // ----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            // ----------------------------------------------------------------
            if (unlikely(ips_mock)) return; // UT

            if (!pds_status) {
                // Delete the L3 VXLAN port from the TEP back reference list
                auto state_ctxt = state_t::thread_context();
                auto tep_obj = state_ctxt.state()->tep_store().get(l_tep_ip);
                tep_obj->del_l3_vxlan_port(vxlan_port_add_upd_ips->id.if_index);
            }
            NBB_CREATE_THREAD_CONTEXT
            NBS_ENTER_SHARED_CONTEXT(li_proc_id);
            NBS_GET_SHARED_DATA();

            auto key = li::VxLanPort::get_key(*vxlan_port_add_upd_ips);
            auto& vxlan_port_store = li::Fte::get().get_lipi_join()->
                                          get_vxlan_port_store();
            auto it = vxlan_port_store.find(key);
            if (likely(it == vxlan_port_store.end())) {
                // MS Stub Stateless mode
                auto send_response =
                    li::VxLanPort::set_ips_rc(&vxlan_port_add_upd_ips->ips_hdr,
                                              (pds_status) ? ATG_OK : ATG_UNSUCCESSFUL);
                SDK_ASSERT(send_response);
                PDS_TRACE_DEBUG("+++++++ Type5 TEP %s VNI %d MS L3 VXLAN Port 0x%x"
                                " Send Async IPS reply %s stateless mode ++++++++",
                                 ipaddr2str(&l_tep_ip), l_vni, key,
                                 (pds_status) ? "Success": "Failure");
                li::Fte::get().get_lipi_join()->
                    send_ips_reply(&vxlan_port_add_upd_ips->ips_hdr);
            } else {
                // MS Stub Stateful mode
                if (pds_status) {
                    PDS_TRACE_DEBUG("Type5 TEP %s VNI %d MS L3 VXLAN Port 0x%x"
                                    " Send Async IPS Reply success stateful mode",
                                     ipaddr2str(&l_tep_ip), l_vni, key);
                    (*it)->update_complete(ATG_OK);
                } else {
                    PDS_TRACE_DEBUG("Type5 TEP %s VNI %d MS L3 VXLAN Port 0x%x"
                                    " Send Async IPS Reply failure stateful mode",
                                     ipaddr2str(&l_tep_ip), l_vni, key);
                    (*it)->update_failed(ATG_UNSUCCESSFUL);
                }
            }
            NBS_RELEASE_SHARED_DATA();
            NBS_EXIT_SHARED_CONTEXT();
            NBB_DESTROY_THREAD_CONTEXT
        };

    // All processing complete, only batch commit remains -
    // safe to release the cookie_uptr_ unique_ptr
    rc = ATG_ASYNC_COMPLETION;
    auto cookie = cookie_uptr_.release();
    auto ret = pds_batch_commit(pds_bctxt_guard.release());
    if (unlikely (ret != SDK_RET_OK)) {
        delete cookie;
        throw Error(std::string("Batch commit failed for Add-Update Type5 TEP ")
                    .append(ipaddr2str(&ips_info_.tep_ip))
                    .append("VNI ").append(std::to_string(ips_info_.vni))
                    .append(" err=").append(std::to_string(ret)));
    }
    PDS_TRACE_DEBUG ("Type5 TEP %s VNI %d Add/Upd PDS Batch commit successful",
                     ipaddr2str(&ips_info_.tep_ip), ips_info_.vni);

    // Add the L3 VXLAN port from the TEP back reference list
    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();
        auto tep_obj = state_ctxt.state()->tep_store().get(ips_info_.tep_ip);
        tep_obj->add_l3_vxlan_port(ips_info_.if_index);
    }
    if (PDS_MOCK_MODE()) {
        // Call the HAL callback in PDS mock mode
        std::thread cb(pds_ms::hal_callback, SDK_RET_OK, cookie);
        cb.detach();
    }
    return rc;
}

void li_vxlan_port::handle_delete(ms_ifindex_t vxlan_port_ifindex) {
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    op_delete_ = true;

    // MS stub Integration APIs do not support Async callback for deletes.
    // However since we should not block the MS NBase main thread
    // the HAL processing is always asynchronous even for deletes.
    // Assuming that Deletes never fail the Store is also updated
    // in a synchronous fashion for deletes so that it is in sync
    // if there is a subsequent create from MS.

    ips_info_.if_index = vxlan_port_ifindex;
    // Empty cookie to force async PDS.
    cookie_uptr_.reset (new cookie_t);
    ip_addr_t tep_ip;

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();
        fetch_store_info_(state_ctxt.state());

        if (store_info_.vxp_if_obj == nullptr) {
            PDS_TRACE_DEBUG("Ignoring possible L2 VXLAN Port 0x%x Delete", vxlan_port_ifindex);
            return;
        }

        tep_ip = store_info_.vxp_if_obj->vxlan_port_properties().tep_ip;
        PDS_TRACE_INFO ("Type5 TEP %s MS L3 VXLAN Port 0x%x: Delete IPS",
                        ipaddr2str(&tep_ip), vxlan_port_ifindex);

        pds_bctxt_guard = make_batch_pds_spec_();

        // Delete the L3 VXLAN port from the TEP back reference list
        store_info_.tep_obj->del_l3_vxlan_port(ips_info_.if_index);

        // If we have batched multiple IPS earlier flush it now
        // VXLAN Tunnel deletion cannot be appended to an existing batch
        state_ctxt.state()->flush_outstanding_pds_batch();

    } // End of state thread_context
      // Do Not access/modify global state after this

    cookie_uptr_->send_ips_reply =
        [tep_ip, vxlan_port_ifindex] (bool pds_status, bool ips_mock) -> void {
            // ----------------------------------------------------------------
            // This block is executed asynchronously when PDS response is rcvd
            // ----------------------------------------------------------------
            PDS_TRACE_DEBUG("+++++++ Type5 TEP %s MS L3 VXLAN Port 0x%x Delete"
                            " Rcvd Async PDS response %s +++++++",
                            ipaddr2str(&tep_ip), vxlan_port_ifindex,
                            (pds_status)?"Success": "Failure");

        };

    // All processing complete, only batch commit remains -
    // safe to release the cookie_uptr_ unique_ptr
    auto cookie = cookie_uptr_.release();
    auto ret = pds_batch_commit(pds_bctxt_guard.release());
    if (unlikely (ret != SDK_RET_OK)) {
        delete cookie;
        throw Error(std::string("Batch commit failed for delete Type5 TEP ")
                    .append(ipaddr2str(&tep_ip))
                    .append(" err=").append(std::to_string(ret)));
    }
    PDS_TRACE_DEBUG ("Type5 TEP %s MS L3 VXLAN Port 0x%x: Delete PDS Batch commit successful",
                     ipaddr2str(&tep_ip), ips_info_.if_index);

    { // Enter thread-safe context to access/modify global state
        auto state_ctxt = pds_ms::state_t::thread_context();
        // Deletes are synchronous - Delete the store entry immediately
        state_ctxt.state()->if_store().erase(ips_info_.if_index);
    }
}

} // End namespace

