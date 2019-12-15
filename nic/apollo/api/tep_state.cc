//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// TEP database handling
///
//----------------------------------------------------------------------------

#include <stdio.h>
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/tep_state.hpp"

using sdk::lib::ht;

namespace api {

/// \defgroup PDS_TEP_STATE - tep database functionality
/// \ingroup PDS_TEP
/// \@{

tep_state::tep_state() {
    tep_ht_ = ht::factory(PDS_MAX_TEP >> 2,
                          tep_entry::tep_key_func_get,
                          sizeof(pds_tep_key_t));
    SDK_ASSERT(tep_ht_ != NULL);

    tep_slab_ = slab::factory("tep", PDS_SLAB_ID_TEP, sizeof(tep_entry),
                              16, true, true, true, NULL);
    SDK_ASSERT(tep_slab_ != NULL);
}

tep_state::~tep_state() {
    ht::destroy(tep_ht_);
    slab::destroy(tep_slab_);
}

tep_entry *
tep_state::alloc(void) {
    return ((tep_entry *)tep_slab_->alloc());
}

sdk_ret_t
tep_state::insert(tep_entry *tep) {
    PDS_TRACE_VERBOSE("Adding TEP %s to db", tep->key2str());
    return tep_ht_->insert_with_key(&tep->key_, tep, &tep->ht_ctxt_);
}

tep_entry *
tep_state::remove(tep_entry *tep) {
    PDS_TRACE_VERBOSE("Deleting TEP %s from db", tep->key2str());
    return (tep_entry *)(tep_ht_->remove(&tep->key_));
}

void
tep_state::free(tep_entry *tep) {
    tep_slab_->free(tep);
}

tep_entry *
tep_state::find(pds_tep_key_t *tep_key) const {
    return (tep_entry *)(tep_ht_->lookup(tep_key));
}

sdk_ret_t
tep_state::walk(state_walk_cb_t walk_cb, void *ctxt) {
    return tep_ht_->walk(walk_cb, ctxt);
}

sdk_ret_t
tep_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(tep_slab_, ctxt);
    return SDK_RET_OK;
}

/// \@}    // end of PDS_TEP_STATE

}    // namespace api
