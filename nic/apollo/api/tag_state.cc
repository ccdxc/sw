//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// tag state handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/tag_state.hpp"

namespace api {

/// \defgroup PDS_TAG_STATE - tag database functionality
/// \ingroup PDS_TAG
/// \@{

tag_state::tag_state() {
    // TODO: need to tune multi-threading related params later
    tag_ht_ = ht::factory(PDS_MAX_TAG >> 1,
                          tag_entry::tag_key_func_get,
                          tag_entry::tag_hash_func_compute,
                          tag_entry::tag_key_func_compare);
    SDK_ASSERT(tag_ht() != NULL);

    tag_slab_ = slab::factory("tag", PDS_SLAB_ID_TAG, sizeof(tag_entry), 16,
                              true, true, NULL);
    SDK_ASSERT(tag_slab() != NULL);
}

tag_state::~tag_state() {
    ht::destroy(tag_ht());
    slab::destroy(tag_slab());
}

tag_entry *
tag_state::alloc(void) {
    return ((tag_entry *)tag_slab()->alloc());
}

sdk_ret_t
tag_state::insert(tag_entry *tag) {
    return tag_ht()->insert_with_key(&tag->key_, tag, &tag->ht_ctxt_);
}

tag_entry *
tag_state::remove(tag_entry *tag) {
    return (tag_entry *)(tag_ht()->remove(&tag->key_));
}

void
tag_state::free(tag_entry *tag) {
    tag_slab()->free(tag);
}

tag_entry *
tag_state::find(pds_tag_key_t *key) const {
    return (tag_entry *)(tag_ht()->lookup(key));
}

/// \@}    // end of PDS_TAG_STATE

}    // namespace api
