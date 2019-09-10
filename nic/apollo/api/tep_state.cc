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
#include "nic/apollo/api/tep_state.hpp"

using sdk::lib::ht;

namespace api {

/// \defgroup PDS_TEP_STATE - tep database functionality
/// \ingroup PDS_TEP
/// \@{

tep_state::tep_state() {
    // TODO: need to tune multi-threading related params later
    tep_ht_ = ht::factory(PDS_MAX_TEP >> 2,
                          tep_entry::tep_key_func_get,
                          tep_entry::key_size());
    SDK_ASSERT(tep_ht() != NULL);

    tep_slab_ = slab::factory("tep", PDS_SLAB_ID_TEP, sizeof(tep_entry),
                              16, true, true, true, NULL);
    SDK_ASSERT(tep_slab() != NULL);
}

tep_state::~tep_state() {
    ht::destroy(tep_ht());
    slab::destroy(tep_slab());
}

tep_entry *
tep_state::alloc(void) {
    return ((tep_entry *)tep_slab()->alloc());
}

sdk_ret_t
tep_state::insert(tep_entry *tep) {
    return tep_ht()->insert_with_key(&tep->key_, tep, &tep->ht_ctxt_);
}

tep_entry *
tep_state::remove(tep_entry *tep) {
    return (tep_entry *)(tep_ht()->remove(&tep->key_));
}

void
tep_state::free(tep_entry *tep) {
    tep_slab()->free(tep);
}

tep_entry *
tep_state::find(pds_tep_key_t *tep_key) const {
    return (tep_entry *)(tep_ht()->lookup(tep_key));
}

/// \@}    // end of PDS_TEP_STATE

}    // namespace api
