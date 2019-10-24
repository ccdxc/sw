//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// nexthop state handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/nexthop_state.hpp"

namespace api {

/// \defgroup PDS_NEXTHOP_STATE - nexthop database functionality
/// \ingroup PDS_NEXTHOP
/// \@{

nexthop_state::nexthop_state() {
    nexthop_ht_ = ht::factory(PDS_MAX_NEXTHOP >> 1,
                              nexthop::nexthop_key_func_get,
                              nexthop::key_size());
    SDK_ASSERT(nexthop_ht_ != NULL);
    nexthop_slab_ = slab::factory("nexthop", PDS_SLAB_ID_NEXTHOP,
                                  sizeof(nexthop), 256, true, true, NULL);
    SDK_ASSERT(nexthop_slab_ != NULL);
}

nexthop_state::~nexthop_state() {
    ht::destroy(nexthop_ht_);
    slab::destroy(nexthop_slab_);
}

nexthop *
nexthop_state::alloc(void) {
    return ((nexthop *)nexthop_slab_->alloc());
}

sdk_ret_t
nexthop_state::insert(nexthop *nh) {
    return nexthop_ht_->insert_with_key(&nh->key_, nh, &nh->ht_ctxt_);
}

nexthop *
nexthop_state::remove(nexthop *nh) {
    return (nexthop *)(nexthop_ht_->remove(&nh->key_));
}

void
nexthop_state::free(nexthop *nh) {
    nexthop_slab_->free(nh);
}

nexthop *
nexthop_state::find(pds_nexthop_key_t *key) const {
    return (nexthop *)(nexthop_ht_->lookup(key));
}

sdk_ret_t
nexthop_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(nexthop_slab_, ctxt);
    return SDK_RET_OK;
}

/// \@}    // end of PDS_NEXTHOP_STATE

}    // namespace api
