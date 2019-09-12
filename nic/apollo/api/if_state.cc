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
    // TODO: need to tune multi-threading related params later
    if_ht_ = ht::factory(PDS_MAX_IF >> 1, if_entry::if_key_func_get,
                         if_entry::key_size());
    SDK_ASSERT(if_ht() != NULL);
    if_slab_ = slab::factory("interface", PDS_SLAB_ID_IF, sizeof(if_entry), 16,
                             true, true, NULL);
    SDK_ASSERT(if_slab() != NULL);
}

if_state::~if_state() {
    ht::destroy(if_ht());
    slab::destroy(if_slab());
}

if_entry *
if_state::alloc(void) {
    return ((if_entry *)if_slab()->alloc());
}

sdk_ret_t
if_state::insert(if_entry *intf) {
    PDS_TRACE_DEBUG("Inserting IF 0x%x", intf->key_);
    return if_ht()->insert_with_key(&intf->key_, intf, &intf->ht_ctxt_);
}

if_entry *
if_state::remove(if_entry *intf) {
    return (if_entry *)(if_ht()->remove(&intf->key_));
}

void
if_state::free(if_entry *intf) {
    if_slab()->free(intf);
}

if_entry *
if_state::find(pds_ifindex_t *key) const {
    return (if_entry *)(if_ht()->lookup(key));
}

bool
if_state::walk_cb_(void *entry, void *ctxt) {
    if_entry *intf = (if_entry *)entry;
    if_walk_ctxt_t *walk_ctxt = (if_walk_ctxt_t *)ctxt;

    if (walk_ctxt->if_type == IF_TYPE_NONE) {
        return walk_ctxt->walk_cb(intf, walk_ctxt->ctxt);
    } else if (walk_ctxt->if_type == IFINDEX_TO_IFTYPE(intf->key())) {
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
    if_ht()->walk(if_state::walk_cb_, &walk_ctxt);
    return SDK_RET_OK;
}

/// \@}    // end of PDS_IF_STATE

}    // namespace api
