//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// subnet state handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/subnet_state.hpp"

using namespace sdk;

namespace api {

subnet_state::subnet_state() {
    // TODO: need to tune multi-threading related params later
    subnet_ht_ =
        ht::factory(PDS_MAX_SUBNET >> 1, subnet_entry::subnet_key_func_get,
                    subnet_entry::key_size());
    SDK_ASSERT(subnet_ht() != NULL);
    // we should accomodate one extra subnet of type provider/substrate/internet
    subnet_idxr_ = indexer::factory(PDS_MAX_SUBNET + 1);
    SDK_ASSERT(subnet_idxr() != NULL);
    subnet_slab_ = slab::factory("subnet", PDS_SLAB_ID_SUBNET,
                                 sizeof(subnet_entry), 16, true, true, NULL);
    SDK_ASSERT(subnet_slab() != NULL);
}

subnet_state::~subnet_state() {
    ht::destroy(subnet_ht());
    indexer::destroy(subnet_idxr());
    slab::destroy(subnet_slab());
}

subnet_entry *
subnet_state::alloc(void) {
    return ((subnet_entry *)subnet_slab()->alloc());
}

sdk_ret_t
subnet_state::insert(subnet_entry *subnet) {
    return subnet_ht()->insert_with_key(&subnet->key_, subnet,
                                        &subnet->ht_ctxt_);
}

subnet_entry *
subnet_state::remove(subnet_entry *subnet) {
    return (subnet_entry *)(subnet_ht()->remove(&subnet->key_));
}

void
subnet_state::free(subnet_entry *subnet) {
    subnet_slab()->free(subnet);
}

subnet_entry *
subnet_state::find(pds_subnet_key_t *subnet_key) const {
    return (subnet_entry *)(subnet_ht()->lookup(subnet_key));
}

}    // namespace api
