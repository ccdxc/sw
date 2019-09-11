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
                    nexthop_group::nexthop_group_hash_func_compute,
                    nexthop_group::nexthop_group_key_func_compare);
    SDK_ASSERT(nh_group_ht() != NULL);
    nexthop_group_slab_ =
        slab::factory("nexthop-group", PDS_SLAB_ID_NEXTHOP_GROUP,
                      sizeof(nexthop_group), 256, true, true, NULL);
    SDK_ASSERT(nh_group_slab() != NULL);
}

nexthop_group_state::~nexthop_group_state() {
    ht::destroy(nh_group_ht());
    slab::destroy(nh_group_slab());
}

nexthop_group *
nexthop_group_state::alloc(void) {
    return ((nexthop_group *)nh_group_slab()->alloc());
}

sdk_ret_t
nexthop_group_state::insert(nexthop_group *nh) {
    return nh_group_ht()->insert_with_key(&nh->key_, nh, &nh->ht_ctxt_);
}

nexthop_group *
nexthop_group_state::remove(nexthop_group *nh) {
    return (nexthop_group *)(nh_group_ht()->remove(&nh->key_));
}

void
nexthop_group_state::free(nexthop_group *nh) {
    nh_group_slab()->free(nh);
}

nexthop_group *
nexthop_group_state::find(pds_nexthop_key_t *key) const {
    return (nexthop_group *)(nh_group_ht()->lookup(key));
}

/// \@}    // end of PDS_NEXTHOP_GROUP_STATE

}    // namespace api
