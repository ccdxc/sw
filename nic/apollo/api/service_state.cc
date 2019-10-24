/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    service_state.cc
 *
 * @brief   service mapping databse handling
 */

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/service_state.hpp"

namespace api {

/**
 * @defgroup PDS_SVC_MAPPING_STATE - mservice apping database functionality
 * @ingroup PDS_SVC_MAPPING
 * @{
 */

svc_mapping_state::svc_mapping_state() {
    svc_mapping_slab_ = slab::factory("svc-mapping", PDS_SLAB_ID_SVC_MAPPING,
                                      sizeof(svc_mapping), 64, true,
                                      true, NULL);
    SDK_ASSERT(svc_mapping_slab_ != NULL);
}

svc_mapping_state::~svc_mapping_state() {
    slab::destroy(svc_mapping_slab_);
}

svc_mapping *
svc_mapping_state::alloc(void) {
    return ((svc_mapping *)svc_mapping_slab_->alloc());
}

void
svc_mapping_state::free(svc_mapping *mapping) {
    svc_mapping_slab_->free(mapping);
}

sdk_ret_t
svc_mapping_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(svc_mapping_slab_, ctxt);
    return SDK_RET_OK;
}

/** @} */    // end of PDS_SVC_MAPPING_STATE

}    // namespace api
