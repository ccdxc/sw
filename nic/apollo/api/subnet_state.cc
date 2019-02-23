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
        ht::factory(OCI_MAX_SUBNET >> 1, subnet_entry::subnet_key_func_get,
                    subnet_entry::subnet_hash_func_compute,
                    subnet_entry::subnet_key_func_compare);
    SDK_ASSERT(subnet_ht_ != NULL);
    subnet_idxr_ = indexer::factory(OCI_MAX_SUBNET);
    SDK_ASSERT(subnet_idxr_ != NULL);
    subnet_slab_ = slab::factory("subnet", OCI_SLAB_ID_SUBNET,
                                 sizeof(subnet_entry), 16, true, true, NULL);
    SDK_ASSERT(subnet_slab_ != NULL);
}

subnet_state::~subnet_state() {
    ht::destroy(subnet_ht_);
    indexer::destroy(subnet_idxr_);
    slab::destroy(subnet_slab_);
}

subnet_entry *
subnet_state::subnet_alloc(void) {
    return ((subnet_entry *)subnet_slab_->alloc());
}

void
subnet_state::subnet_free(subnet_entry *subnet) {
    subnet_slab_->free(subnet);
}

subnet_entry *
subnet_state::subnet_find(oci_subnet_key_t *subnet_key) const {
    return (subnet_entry *)(subnet_ht_->lookup(subnet_key));
}

}    // namespace api
