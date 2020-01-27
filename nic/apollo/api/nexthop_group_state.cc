//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// nexthop group state handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/nexthop_group_state.hpp"

namespace api {

/// \defgroup PDS_NEXTHOP_GROUP_STATE - nexthop group database functionality
/// \ingroup PDS_NEXTHOP
/// \@{

nexthop_group_state::nexthop_group_state() {
    nexthop_group_ht_ =
        ht::factory(PDS_MAX_NEXTHOP_GROUP >> 1,
                    nexthop_group::nexthop_group_key_func_get,
                    sizeof(pds_obj_key_t));
    SDK_ASSERT(nexthop_group_ht_ != NULL);
    nexthop_group_slab_ =
        slab::factory("nexthop-group", PDS_SLAB_ID_NEXTHOP_GROUP,
                      sizeof(nexthop_group), 256, true, true, NULL);
    SDK_ASSERT(nexthop_group_slab_ != NULL);
}

nexthop_group_state::~nexthop_group_state() {
    ht::destroy(nexthop_group_ht_);
    slab::destroy(nexthop_group_slab_);
}

nexthop_group *
nexthop_group_state::alloc(void) {
    return ((nexthop_group *)nexthop_group_slab_->alloc());
}

sdk_ret_t
nexthop_group_state::insert(nexthop_group *nh_group) {
    PDS_TRACE_VERBOSE("Adding nexthop group %s to db", nh_group->key().str());
    return nexthop_group_ht_->insert_with_key(&nh_group->key_,
                                              nh_group, &nh_group->ht_ctxt_);
}

nexthop_group *
nexthop_group_state::remove(nexthop_group *nh_group) {
    PDS_TRACE_VERBOSE("Removing nexthop group %s from db", nh_group->key().str());
    return (nexthop_group *)(nexthop_group_ht_->remove(&nh_group->key_));
}

void
nexthop_group_state::free(nexthop_group *nh) {
    nexthop_group_slab_->free(nh);
}

nexthop_group *
nexthop_group_state::find(pds_obj_key_t *key) const {
    return (nexthop_group *)(nexthop_group_ht_->lookup(key));
}

sdk_ret_t
nexthop_group_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(nexthop_group_slab_, ctxt);
    return SDK_RET_OK;
}

/// \@}    // end of PDS_NEXTHOP_GROUP_STATE

}    // namespace api
