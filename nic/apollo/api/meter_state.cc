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
                            sizeof(pds_obj_key_t));
    SDK_ASSERT(meter_ht() != NULL);

    meter_slab_ = slab::factory("meter", PDS_SLAB_ID_METER, sizeof(meter_entry),
                                16, true, true, NULL);
    SDK_ASSERT(meter_slab() != NULL);
}

meter_state::~meter_state() {
    ht::destroy(meter_ht());
    slab::destroy(meter_slab());
}

meter_entry *
meter_state::alloc(void) {
    return ((meter_entry *)meter_slab()->alloc());
}

sdk_ret_t
meter_state::insert(meter_entry *meter) {
    return meter_ht()->insert_with_key(&meter->key_, meter, &meter->ht_ctxt_);
}

meter_entry *
meter_state::remove(meter_entry *meter) {
    return (meter_entry *)(meter_ht()->remove(&meter->key_));
}

void
meter_state::free(meter_entry *meter) {
    meter_slab()->free(meter);
}

meter_entry *
meter_state::find(pds_obj_key_t *key) const {
    return (meter_entry *)(meter_ht()->lookup(key));
}

sdk_ret_t
meter_state::walk(state_walk_cb_t walk_cb, void *ctxt) {
    return meter_ht()->walk(walk_cb, ctxt);
}

/// \@}    // end of PDS_METER_STATE

}    // namespace api
