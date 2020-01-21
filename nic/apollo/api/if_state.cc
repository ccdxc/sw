//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// interface state handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/apollo/api/if_state.hpp"
#include "nic/apollo/core/trace.hpp"

namespace api {

/// \defgroup PDS_IF_STATE - interface database functionality
/// \ingroup PDS_IF
/// \@{

if_state::if_state() {
    ifindex_ht_ = ht::factory(PDS_MAX_IF >> 1, if_entry::ifindex_func_get,
                              sizeof(pds_ifindex_t));
    SDK_ASSERT(ifindex_ht_ != NULL);
    if_ht_ = ht::factory(PDS_MAX_IF >> 1, if_entry::if_key_func_get,
                         sizeof(pds_obj_key_t));
    SDK_ASSERT(if_ht_ != NULL);
    if_slab_ = slab::factory("interface", PDS_SLAB_ID_IF, sizeof(if_entry), 16,
                             true, true, true, NULL);
    SDK_ASSERT(if_slab() != NULL);
}

if_state::~if_state() {
    ht::destroy(ifindex_ht_);
    ht::destroy(if_ht_);
    slab::destroy(if_slab_);
}

if_entry *
if_state::alloc(void) {
    return ((if_entry *)if_slab_->alloc());
}

sdk_ret_t
if_state::insert(if_entry *intf) {
    sdk_ret_t ret;

    PDS_TRACE_VERBOSE("Inserting interface %s, ifindex 0x%x",
                      intf->key_.str(), intf->ifindex_);
    ret = ifindex_ht_->insert_with_key(&intf->ifindex_, intf,
                                       &intf->ifindex_ht_ctxt_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to insert interface %s, ifindex 0x%x into "
                      "if db, err %u", intf->key_.str(), intf->ifindex_, ret);
        return ret;
    }
    return if_ht_->insert_with_key(&intf->key_, intf, &intf->ht_ctxt_);
}

if_entry *
if_state::remove(if_entry *intf) {
    ifindex_ht_->remove(&intf->ifindex_);
    return (if_entry *)(if_ht_->remove(&intf->key_));
}

void
if_state::free(if_entry *intf) {
    if_slab()->free(intf);
}

if_entry *
if_state::find(pds_ifindex_t *ifindex) const {
    return (if_entry *)(ifindex_ht_->lookup(ifindex));
}

if_entry *
if_state::find(pds_obj_key_t *key) const {
    return (if_entry *)(if_ht_->lookup(key));
}

bool
if_state::walk_cb_(void *entry, void *ctxt) {
    if_entry *intf = (if_entry *)entry;
    if_walk_ctxt_t *walk_ctxt = (if_walk_ctxt_t *)ctxt;

    if (walk_ctxt->if_type == IF_TYPE_NONE) {
        return walk_ctxt->walk_cb(intf, walk_ctxt->ctxt);
    } else if (walk_ctxt->if_type == IFINDEX_TO_IFTYPE(intf->ifindex())) {
        return walk_ctxt->walk_cb(intf, walk_ctxt->ctxt);
    }
    return false;
}

sdk_ret_t
if_state::walk(uint32_t if_type, sdk::lib::ht::ht_walk_cb_t walk_cb,
               void *ctxt) {
    if_walk_ctxt_t walk_ctxt;

    walk_ctxt.if_type = if_type;
    walk_ctxt.ctxt = ctxt;
    walk_ctxt.walk_cb = walk_cb;
    ifindex_ht_->walk(if_state::walk_cb_, &walk_ctxt);
    return SDK_RET_OK;
}

sdk_ret_t
if_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(if_slab_, ctxt);
    return SDK_RET_OK;
}

/// \@}    // end of PDS_IF_STATE

}    // namespace api
