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

// service mapping hash table size in s/w
// NOTE: this has no relation to max# of service mappings, it should be big
//       enough to accomodate all svc mapping API calls in a batch efficiently
//       and because service mappings are stateless objects, this hash table
//       will be empty once a API batch is processed
#define PDS_SVC_MAPPING_HT_SIZE    1024

svc_mapping_state::svc_mapping_state() {
    svc_mapping_ht_ = ht::factory(PDS_SVC_MAPPING_HT_SIZE,
                                  svc_mapping::svc_mapping_key_func_get,
                                  sizeof(pds_svc_mapping_key_t));
    SDK_ASSERT(svc_mapping_ht_ != NULL);
    svc_mapping_slab_ = slab::factory("svc-mapping", PDS_SLAB_ID_SVC_MAPPING,
                                      sizeof(svc_mapping), 64, true, true,
                                      true, NULL);
    SDK_ASSERT(svc_mapping_slab_ != NULL);
}

svc_mapping_state::~svc_mapping_state() {
    ht::destroy(svc_mapping_ht_);
    slab::destroy(svc_mapping_slab_);
}

svc_mapping *
svc_mapping_state::alloc(void) {
    return ((svc_mapping *)svc_mapping_slab_->alloc());
}

sdk_ret_t
svc_mapping_state::insert(svc_mapping *mapping) {
    return svc_mapping_ht_->insert_with_key(&mapping->key_, mapping,
                                            &mapping->ht_ctxt_);
}

svc_mapping *
svc_mapping_state::remove(svc_mapping *mapping) {
    return (svc_mapping *)(svc_mapping_ht_->remove(&mapping->key_));
}

void
svc_mapping_state::free(svc_mapping *mapping) {
    svc_mapping_slab_->free(mapping);
}

svc_mapping *
svc_mapping_state::find(pds_svc_mapping_key_t *key) const {
    return (svc_mapping *)(svc_mapping_ht_->lookup(key));
}

sdk_ret_t
svc_mapping_state::walk(state_walk_cb_t walk_cb, void *ctxt) {
    return svc_mapping_ht_->walk(walk_cb, ctxt);
}

sdk_ret_t
svc_mapping_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(svc_mapping_slab_, ctxt);
    return SDK_RET_OK;
}

/** @} */    // end of PDS_SVC_MAPPING_STATE

}    // namespace api
