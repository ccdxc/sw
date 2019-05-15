//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// meter state handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/meter_state.hpp"

namespace api {

/// \defgroup PDS_METER_STATE - meter database functionality
/// \ingroup PDS_METER
/// \@{

meter_state::meter_state() {
    // TODO: need to tune multi-threading related params later
    meter_ht_ = ht::factory(PDS_MAX_METER >> 1,
                          meter_entry::meter_key_func_get,
                          meter_entry::meter_hash_func_compute,
                          meter_entry::meter_key_func_compare);
    SDK_ASSERT(meter_ht_ != NULL);

    meter_slab_ = slab::factory("meter", PDS_SLAB_ID_METER, sizeof(meter_entry),
                                16, true, true, NULL);
    SDK_ASSERT(meter_slab_ != NULL);
}

meter_state::~meter_state() {
    ht::destroy(meter_ht_);
    slab::destroy(meter_slab_);
}

meter_entry *
meter_state::alloc(void) {
    return ((meter_entry *)meter_slab_->alloc());
}

sdk_ret_t
meter_state::insert(meter_entry *meter) {
    return meter_ht_->insert_with_key(&meter->key_, meter, &meter->ht_ctxt_);
}

meter_entry *
meter_state::remove(meter_entry *meter) {
    return (meter_entry *)(meter_ht_->remove(&meter->key_));
}

void
meter_state::free(meter_entry *meter) {
    meter_slab_->free(meter);
}

meter_entry *
meter_state::find(pds_meter_key_t *key) const {
    return (meter_entry *)(meter_ht_->lookup(key));
}

/// \@}    // end of PDS_METER_STATE

}    // namespace api
