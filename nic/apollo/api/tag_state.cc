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
                          sizeof(pds_tag_key_t));
    SDK_ASSERT(tag_ht_ != NULL);

    tag_slab_ = slab::factory("tag", PDS_SLAB_ID_TAG, sizeof(tag_entry), 16,
                              true, true, NULL);
    SDK_ASSERT(tag_slab_ != NULL);
}

tag_state::~tag_state() {
    ht::destroy(tag_ht_);
    slab::destroy(tag_slab_);
}

tag_entry *
tag_state::alloc(void) {
    return ((tag_entry *)tag_slab_->alloc());
}

sdk_ret_t
tag_state::insert(tag_entry *tag) {
    return tag_ht_->insert_with_key(&tag->key_, tag, &tag->ht_ctxt_);
}

tag_entry *
tag_state::remove(tag_entry *tag) {
    return (tag_entry *)(tag_ht_->remove(&tag->key_));
}

void
tag_state::free(tag_entry *tag) {
    tag_slab_->free(tag);
}

tag_entry *
tag_state::find(pds_tag_key_t *key) const {
    return (tag_entry *)(tag_ht_->lookup(key));
}

sdk_ret_t
tag_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(tag_slab_, ctxt);
    return SDK_RET_OK;
}

/// \@}    // end of PDS_TAG_STATE

}    // namespace api
