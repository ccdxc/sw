/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    mapping_state.cc
 *
 * @brief   mapping databse handling
 */

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/mapping_state.hpp"

namespace api {

/**
 * @defgroup PDS_MAPPING_STATE - mapping database functionality
 * @ingroup PDS_MAPPING
 * @{
 */

mapping_state::mapping_state() {
    mapping_slab_ = slab::factory("mapping", PDS_SLAB_ID_MAPPING,
                                  sizeof(mapping_entry),
                                  8192, true, true, NULL);
    SDK_ASSERT(mapping_slab_ != NULL);
}

mapping_state::~mapping_state() {
    slab::destroy(mapping_slab_);
}

mapping_entry *
mapping_state::alloc(void) {
    return ((mapping_entry *)mapping_slab_->alloc());
}

void
mapping_state::free(mapping_entry *mapping) {
    mapping_slab_->free(mapping);
}

sdk_ret_t
mapping_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(mapping_slab_, ctxt);
    return SDK_RET_OK;
}

/** @} */    // end of PDS_MAPPING_STATE

}    // namespace api
