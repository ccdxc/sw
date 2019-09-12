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
    SDK_ASSERT(nh_ht() != NULL);
    nexthop_slab_ = slab::factory("nexthop", PDS_SLAB_ID_NEXTHOP,
                                  sizeof(nexthop), 256, true, true, NULL);
    SDK_ASSERT(nh_slab() != NULL);
}

nexthop_state::~nexthop_state() {
    ht::destroy(nh_ht());
    slab::destroy(nh_slab());
}

nexthop *
nexthop_state::alloc(void) {
    return ((nexthop *)nh_slab()->alloc());
}

sdk_ret_t
nexthop_state::insert(nexthop *nh) {
    return nh_ht()->insert_with_key(&nh->key_, nh, &nh->ht_ctxt_);
}

nexthop *
nexthop_state::remove(nexthop *nh) {
    return (nexthop *)(nh_ht()->remove(&nh->key_));
}

void
nexthop_state::free(nexthop *nh) {
    nh_slab()->free(nh);
}

nexthop *
nexthop_state::find(pds_nexthop_key_t *key) const {
    return (nexthop *)(nh_ht()->lookup(key));
}

/// \@}    // end of PDS_NEXTHOP_STATE

}    // namespace api
