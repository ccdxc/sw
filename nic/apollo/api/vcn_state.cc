//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vcn state handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/vcn_state.hpp"

namespace api {

vcn_state::vcn_state() {
    // TODO: need to tune multi-threading related params later
    vcn_ht_ = ht::factory(OCI_MAX_VCN >> 1, vcn_entry::vcn_key_func_get,
                          vcn_entry::vcn_hash_func_compute,
                          vcn_entry::vcn_key_func_compare);
    SDK_ASSERT(vcn_ht_ != NULL);
    vcn_idxr_ = indexer::factory(OCI_MAX_VCN);
    SDK_ASSERT(vcn_idxr_ != NULL);
    vcn_slab_ = slab::factory("vcn", OCI_SLAB_ID_VCN, sizeof(vcn_entry), 16,
                              true, true, NULL);
    SDK_ASSERT(vcn_slab_ != NULL);
}

vcn_state::~vcn_state() {
    ht::destroy(vcn_ht_);
    indexer::destroy(vcn_idxr_);
    slab::destroy(vcn_slab_);
}

vcn_entry *
vcn_state::vcn_alloc(void) {
    return ((vcn_entry *)vcn_slab_->alloc());
}

void
vcn_state::vcn_free(vcn_entry *vcn) {
    vcn_slab_->free(vcn);
}

vcn_entry *
vcn_state::vcn_find(oci_vcn_key_t *vcn_key) const {
    return (vcn_entry *)(vcn_ht_->lookup(vcn_key));
}

}    // namespace api
