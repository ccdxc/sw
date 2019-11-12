//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// LI VXLAN Tunnel HAL integration
//---------------------------------------------------------------

#include "pdsa_li_vxlan_tnl.hpp"
#include "nic/sdk/lib/logger/logger.hpp"

namespace pdsa_stub {

void 
li_vxlan_tnl::cache_obj_in_cookie_for_create_op_ (void)
{
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
li_vxlan_tnl::cache_obj_in_cookie_for_update_op_ (void)
{
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
li_vxlan_tnl::cache_obj_in_cookie_for_delete_op_ (void)
{
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

pds_batch_ctxt_guard_t
li_vxlan_tnl::make_batch_pds_spec_ ()
{
    pds_batch_ctxt_guard_t bctxt_guard_;

    pds_batch_params_t bp {0, true, (uint64_t) cookie_uptr_.get()};
    auto bctxt = pds_batch_start(&bp);

    if (unlikely (!bctxt)) {
        throw Error {"VxlanAddUpdate PDS Batch Start failed"};
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
            throw Error {"PDS Tep Create failed"};
        }

        auto nhgroup_spec = make_pds_nhgroup_spec_();
#if 0 // TODO: API unavailable
        if (op_create_) {
            ret = pds_nexthop_group_create(&nhgroup_spec, bctxt);
        } else {
            ret = pds_nexthop_group_update(&nhgroup_spec, bctxt);
        }
        if (unlikely (ret != SDK_RET_OK)) {
            throw Error {"PDS ECMP Create failed"};
        }
#else
        nhgroup_spec = nhgroup_spec;
#endif
    }
    return bctxt_guard_;
}

bool
li_vxlan_tnl::handle_add_upd_ips (ATG_LIPI_VXLAN_ADD_UPDATE* vxlan_tnl_add_upd) 
{
    pds_batch_ctxt_guard_t  pds_bctxt_guard;

    // Alloc new cookie and cache IPS
    cookie_uptr_.reset (new cookie_t);
    cookie_uptr_->ips = (NBB_IPS*) vxlan_tnl_add_upd;

    parse_ips_info_(vxlan_tnl_add_upd);

    auto tep_ip_str = ipaddr2str(&ips_info_.tep_ip);

    { // Enter thread-safe context to access/modify global state
    auto state_ctxt = pdsa_stub::state_t::thread_context();

    fetch_store_info_(state_ctxt.state());

    if (store_info_.tep_obj != nullptr) {
        // Update Tunnel
        SDK_TRACE_INFO ("TEP %s: Update IPS", tep_ip_str);
        if (unlikely(!cache_obj_in_cookie_for_update_op_())) {
            // No change
            return false;
        } 
        pds_bctxt_guard = make_batch_pds_spec_(); 
    } else {
        // Create Tunnel
        SDK_TRACE_INFO ("TEP %s: Create IPS", tep_ip_str);
        op_create_ = true;
        cache_obj_in_cookie_for_create_op_(); 
        pds_bctxt_guard = make_batch_pds_spec_(); 
    }

    // If we have batched multiple IPS earlier flush it now
    // Cannot add a Tunnel create to an existing batch
    state_ctxt.state()->flush_outstanding_pds_batch();

    } // End of state thread_context
      // Do Not access/modify global state after this

    // All processing complete, only batch commit remains - safe to release the cookie_uptr_ unique_ptr
    cookie_uptr_.release();
    pds_batch_commit(pds_bctxt_guard.release());
    SDK_TRACE_DEBUG ("TEP %s: Add/Upd PDS Batch commit successful", tep_ip_str);
    return true;
}

bool
li_vxlan_tnl::handle_delete (NBB_ULONG tnl_ifindex) 
{
    pds_batch_ctxt_guard_t  pds_bctxt_guard;
    op_delete_ = true;
    char* tep_ip_str;

    // Alloc new cookie without IPS (sync response to MS)
    cookie_uptr_.reset (new cookie_t);
    cookie_uptr_->op_delete = true;

    ips_info_.if_index = tnl_ifindex;
    { // Enter thread-safe context to access/modify global state
    auto state_ctxt = pdsa_stub::state_t::thread_context();

    fetch_store_info_(state_ctxt.state());
    if (unlikely (store_info_.tep_obj == nullptr && 
                  store_info_.tun_if_obj == nullptr)) {
        // No change
        return ATG_OK;
    }
    tep_ip_str = ipaddr2str(&ips_info_.tep_ip);
    SDK_TRACE_INFO ("TEP %s: Delete IPS", tep_ip_str);
    // Delete Tunnel
    cache_obj_in_cookie_for_delete_op_(); 
    pds_bctxt_guard = make_batch_pds_spec_ (); 

    // If we have batched multiple IPS earlier flush it now
    // Cannot add a Tunnel create to an existing batch
    state_ctxt.state()->flush_outstanding_pds_batch();

    } // End of state thread_context
      // Do Not access/modify global state after this

    // All processing complete, only batch commit remains - safe to release the cookie_uptr_ unique_ptr
    cookie_uptr_.release();
    pds_batch_commit(pds_bctxt_guard.release());
    SDK_TRACE_DEBUG ("TEP %s: Delete PDS Batch commit successful", tep_ip_str);
    return true;
}

} // End namespace
