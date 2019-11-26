//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// LI VXLAN Tunnel HAL integration
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/hals/pdsa_li_vxlan_tnl.hpp"
#include "nic/metaswitch/stubs/common/pdsa_util.hpp"
#include "nic/sdk/lib/logger/logger.hpp"

namespace pdsa_stub {

void li_vxlan_tnl::fetch_store_info_(pdsa_stub::state_t* state) {
    store_info_.tun_if_obj = state->if_store().get(ips_info_.if_index);
    if (op_delete_) {
        if (unlikely(store_info_.tun_if_obj == nullptr)) {
            throw Error("VXLAN Tunnel delete for unknown IfIndex " + std::to_string(ips_info_.if_index));
        }
        auto& tun_prop = store_info_.tun_if_obj->vxlan_tunnel_properties();
        store_info_.tep_obj = state->tep_store().get(tun_prop.tep_ip.addr.v4_addr);
        SDK_ASSERT (store_info_.tep_obj != nullptr);
    } else {
        store_info_.tep_obj = state->tep_store().get(ips_info_.tep_ip.addr.v4_addr);
    }
}

pds_tep_spec_t li_vxlan_tnl::make_pds_tep_spec_(void) {
    pds_tep_spec_t spec = {0};
    auto& tep_prop = store_info_.tep_obj->properties();
    spec.key = make_pds_tep_key_();
    spec.remote_ip = tep_prop.tep_ip;
    spec.ip_addr = ips_info_.src_ip;
    spec.nh_type = PDS_NH_TYPE_UNDERLAY_ECMP;
    spec.nh_group.id = tep_prop.hal_uecmp_idx;
    spec.type = PDS_TEP_TYPE_WORKLOAD;
    spec.nat = false;
    return spec;
}

pds_nexthop_group_spec_t li_vxlan_tnl::make_pds_nhgroup_spec_(void) {
    pds_nexthop_group_spec_t spec = {0};
    auto& tep_prop = store_info_.tep_obj->properties();
    spec.key = make_pds_nhgroup_key_();
    spec.type = PDS_NHGROUP_TYPE_OVERLAY_ECMP;
    spec.num_nexthops = 1;
    spec.nexthops[0].key.id = 1; // Unused for NHs pointing to TEPs
    spec.nexthops[0].type = PDS_NH_TYPE_OVERLAY;
    // Use the TEP MS IfIndex as the TEP Index
    spec.nexthops[0].tep.id = tep_prop.hal_tep_idx;
    return spec;
}

void li_vxlan_tnl::parse_ips_info_(ATG_LIPI_VXLAN_ADD_UPDATE* vxlan_tnl_add_upd) {
    ips_info_.if_index = vxlan_tnl_add_upd->id.if_index;
    ATG_INET_ADDRESS& ms_dest_ip = vxlan_tnl_add_upd->vxlan_settings.dest_ip;
    pdsa_stub::convert_ipaddr_ms_to_pdsa(ms_dest_ip, &ips_info_.tep_ip);
    ATG_INET_ADDRESS& ms_src_ip = vxlan_tnl_add_upd->vxlan_settings.source_ip;
    pdsa_stub::convert_ipaddr_ms_to_pdsa(ms_src_ip, &ips_info_.src_ip);
    NBB_CORR_GET_VALUE(ips_info_.hal_uecmp_idx, vxlan_tnl_add_upd->id.hw_correlator);
    ips_info_.tep_ip_str = ipaddr2str(&ips_info_.tep_ip);
}

void li_vxlan_tnl::cache_obj_in_cookie_for_create_op_(void) {
    if (likely (store_info_.tun_if_obj == nullptr)) {
        // Create new If Object but do not save it in the Global State yet
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
    } else {
        auto& tnl_if_prop = store_info_.tun_if_obj->vxlan_tunnel_properties();
        // Dest IP cannot change for existing tunnel
        SDK_ASSERT(IPADDR_EQ (&tnl_if_prop.tep_ip, &ips_info_.tep_ip));
    }

    // Create new Tep Object but do not save it in the Global State yet
    // Use the MS Tunnel IfIndex as the HAL index for ECMP and TEP tables
    std::unique_ptr<tep_obj_t> new_tep_obj 
        (new tep_obj_t({ips_info_.tep_ip, ips_info_.hal_uecmp_idx, 
                       ips_info_.if_index, ips_info_.if_index}));
    // Update the local store info context so that the make_pds_spec 
    // refers to the latest fields
    store_info_.tep_obj = new_tep_obj.get(); 
    // Cache the new object in the cookie to revisit asynchronously
    // when the PDS API response is received
    cookie_uptr_->objs.push_back(std::move(new_tep_obj));
}

bool 
li_vxlan_tnl::cache_obj_in_cookie_for_update_op_(void) {
    // Updating existing tunnel - check all properties to see what has changed
    auto& tnl_if_prop = store_info_.tun_if_obj->vxlan_tunnel_properties();
    // Dest IP cannot change for existing tunnel
    SDK_ASSERT(IPADDR_EQ (&tnl_if_prop.tep_ip, &ips_info_.tep_ip));

    if (unlikely (ips_info_.hal_uecmp_idx == 
                  store_info_.tep_obj->properties().hal_uecmp_idx)) {
        // No change in TEP
        return false;
    }

    // Create a new object with the updated fields
    // but do not save it in the Global State yet
    std::unique_ptr<tep_obj_t> new_tep_obj 
        (new tep_obj_t(*(store_info_.tep_obj)));
    new_tep_obj->properties().hal_uecmp_idx = ips_info_.hal_uecmp_idx;
    // Update the local store info context so that the make_pds_spec 
    // refers to the latest fields
    store_info_.tep_obj = new_tep_obj.get(); 
    // Cache the new object in the cookie to revisit asynchronously
    // when the PDS API response is received
    cookie_uptr_->objs.push_back(std::move(new_tep_obj));
    return true;
}

void 
li_vxlan_tnl::cache_obj_in_cookie_for_delete_op_(void) {
    // Do not delete the object from the store until PDS Async response is received
    // Cache a copy of the object in the cookie to revisit and delete asynchronously
    if (store_info_.tep_obj != nullptr) {
        std::unique_ptr<tep_obj_t> new_tep_obj (new tep_obj_t(*(store_info_.tep_obj)));
        cookie_uptr_->objs.push_back(std::move(new_tep_obj));
    }
    if (store_info_.tun_if_obj != nullptr) {
        std::unique_ptr<if_obj_t> new_if_obj (new if_obj_t(*(store_info_.tun_if_obj)));
        cookie_uptr_->objs.push_back(std::move(new_if_obj));
    }
}

pds_batch_ctxt_guard_t li_vxlan_tnl::make_batch_pds_spec_() {
    pds_batch_ctxt_guard_t bctxt_guard_;

    pds_batch_params_t bp {0, true, (uint64_t) cookie_uptr_.get()};
    auto bctxt = pds_batch_start(&bp);

    if (unlikely (!bctxt)) {
        throw Error("PDS Batch Start failed for TEP " 
                    + ips_info_.tep_ip_str);
    }
    bctxt_guard_.set (bctxt);

    if (op_delete_) { // Delete
        auto tep_key = make_pds_tep_key_();
        pds_tep_delete(&tep_key, bctxt);

        auto nhgroup_key = make_pds_nhgroup_key_();
        pds_nexthop_group_delete(&nhgroup_key, bctxt);
    } else { // Add or update
        auto tep_spec = make_pds_tep_spec_();
        sdk_ret_t ret;
        if (op_create_) {
            ret = pds_tep_create(&tep_spec, bctxt);
        } else {
            ret = pds_tep_update(&tep_spec, bctxt);
        }
        if (unlikely (ret != SDK_RET_OK)) {
            throw Error("PDS Tep Create failed for TEP " 
                        + ips_info_.tep_ip_str);
        }

        auto nhgroup_spec = make_pds_nhgroup_spec_();
        if (op_create_) {
            ret = pds_nexthop_group_create(&nhgroup_spec, bctxt);
        } else {
            ret = pds_nexthop_group_update(&nhgroup_spec, bctxt);
        }
        if (unlikely (ret != SDK_RET_OK)) {
            throw Error("PDS ECMP Create failed for TEP "
                        + ips_info_.tep_ip_str);
        }
    }
    return bctxt_guard_;
}

bool li_vxlan_tnl::handle_add_upd_ips(ATG_LIPI_VXLAN_ADD_UPDATE* vxlan_tnl_add_upd) {
    pds_batch_ctxt_guard_t  pds_bctxt_guard;

    parse_ips_info_(vxlan_tnl_add_upd);
    // Alloc new cookie and cache IPS
    cookie_uptr_.reset (new cookie_t);
    cookie_uptr_->ips = (NBB_IPS*) vxlan_tnl_add_upd;

    { // Enter thread-safe context to access/modify global state
    auto state_ctxt = pdsa_stub::state_t::thread_context();

    fetch_store_info_(state_ctxt.state());

    if (store_info_.tep_obj != nullptr) {
        // Update Tunnel
        SDK_TRACE_INFO ("TEP %s: Update IPS", ips_info_.tep_ip_str.c_str());
        if (unlikely(!cache_obj_in_cookie_for_update_op_())) {
            // No change
            return false;
        } 
    } else {
        // Create Tunnel
        SDK_TRACE_INFO ("TEP %s: Create IPS", ips_info_.tep_ip_str.c_str());
        op_create_ = true;
        cache_obj_in_cookie_for_create_op_(); 
    }
    pds_bctxt_guard = make_batch_pds_spec_(); 

    // If we have batched multiple IPS earlier flush it now
    // Cannot add a Tunnel create to an existing batch
    state_ctxt.state()->flush_outstanding_pds_batch();

    } // End of state thread_context
      // Do Not access/modify global state after this

    // All processing complete, only batch commit remains - safe to release the cookie_uptr_ unique_ptr
    auto cookie = cookie_uptr_.release();
    if (pds_batch_commit(pds_bctxt_guard.release()) != SDK_RET_OK) {
        delete cookie;
        throw Error("Batch commit failed for Add-Update TEP " 
                    + ips_info_.tep_ip_str);
    }
    SDK_TRACE_DEBUG ("TEP %s: Add/Upd PDS Batch commit successful", 
                     ips_info_.tep_ip_str.c_str());
    return true;
}

bool li_vxlan_tnl::handle_delete(NBB_ULONG tnl_ifindex) {
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    op_delete_ = true;

    ips_info_.if_index = tnl_ifindex;
    { // Enter thread-safe context to access/modify global state
    auto state_ctxt = pdsa_stub::state_t::thread_context();

    fetch_store_info_(state_ctxt.state());
    if (unlikely (store_info_.tep_obj == nullptr && 
                  store_info_.tun_if_obj == nullptr)) {
        // No change
        return ATG_OK;
    }
    ips_info_.tep_ip_str = ipaddr2str(&store_info_.tep_obj->properties().tep_ip);
    SDK_TRACE_INFO ("TEP %s: Delete IPS", ips_info_.tep_ip_str.c_str());

    // Alloc new cookie without IPS (sync response to MS)
    cookie_uptr_.reset (new cookie_t);
    cookie_uptr_->op_delete = true;

    // Delete Tunnel
    cache_obj_in_cookie_for_delete_op_(); 
    pds_bctxt_guard = make_batch_pds_spec_ (); 

    // If we have batched multiple IPS earlier flush it now
    // Cannot add a Tunnel create to an existing batch
    state_ctxt.state()->flush_outstanding_pds_batch();

    } // End of state thread_context
      // Do Not access/modify global state after this

    // All processing complete, only batch commit remains - safe to release the cookie_uptr_ unique_ptr
    auto cookie = cookie_uptr_.release();
    if (pds_batch_commit(pds_bctxt_guard.release()) != SDK_RET_OK) {
        delete cookie;
        throw Error("Batch commit failed for delete TEP " + ips_info_.tep_ip_str);
    }
    SDK_TRACE_DEBUG ("TEP %s: Delete PDS Batch commit successful", 
                     ips_info_.tep_ip_str.c_str());
    return true;
}

} // End namespace
