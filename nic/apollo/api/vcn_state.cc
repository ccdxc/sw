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
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/vcn_state.hpp"

namespace api {

/// \defgroup PDS_VCN_STATE - vcn database functionality
/// \ingroup PDS_VCN
/// \@{

vcn_state::vcn_state() {
    // TODO: need to tune multi-threading related params later
    vcn_ht_ = ht::factory(PDS_MAX_VCN >> 1, vcn_entry::vcn_key_func_get,
                          vcn_entry::vcn_hash_func_compute,
                          vcn_entry::vcn_key_func_compare);
    SDK_ASSERT(vcn_ht_ != NULL);
    vcn_idxr_ = indexer::factory(PDS_MAX_VCN);
    SDK_ASSERT(vcn_idxr_ != NULL);
    vcn_slab_ = slab::factory("vcn", PDS_SLAB_ID_VCN, sizeof(vcn_entry), 16,
                              true, true, NULL);
    SDK_ASSERT(vcn_slab_ != NULL);
}

vcn_state::~vcn_state() {
    ht::destroy(vcn_ht_);
    indexer::destroy(vcn_idxr_);
    slab::destroy(vcn_slab_);
}

vcn_entry *
vcn_state::alloc(void) {
    return ((vcn_entry *)vcn_slab_->alloc());
}

sdk_ret_t
vcn_state::insert(vcn_entry *vcn) {
    return vcn_ht_->insert_with_key(&vcn->key_, vcn, &vcn->ht_ctxt_);
}

vcn_entry *
vcn_state::remove(vcn_entry *vcn) {
    return (vcn_entry *)(vcn_ht_->remove(&vcn->key_));
}

void
vcn_state::free(vcn_entry *vcn) {
    vcn_slab_->free(vcn);
}

vcn_entry *
vcn_state::find(pds_vcn_key_t *key) const {
    return (vcn_entry *)(vcn_ht_->lookup(key));
}

/// \@}    // end of PDS_VCN_STATE

}    // namespace api
