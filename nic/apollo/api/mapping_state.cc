/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    mapping_state.cc
 *
 * @brief   mapping databse handling
 */

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/mapping_state.hpp"

namespace api {

/**
 * @defgroup PDS_MAPPING_STATE - mapping database functionality
 * @ingroup PDS_MAPPING
 * @{
 */

// mapping hash table size in s/w
// NOTE: this has no relation to max# of mappings, it should be big enough to
// accomodate all mapping API calls in  a batch efficiently and because mappings
// are stateless objects, this hash table will be empty once a API batch is
// processed
#define PDS_MAPPING_HT_SIZE    (128 << 10)

mapping_state::mapping_state() {
    mapping_ht_ = ht::factory(PDS_MAPPING_HT_SIZE,
                              mapping_entry::mapping_key_func_get,
                              sizeof(pds_mapping_key_t));
    SDK_ASSERT(mapping_ht_ != NULL);
    mapping_slab_ = slab::factory("mapping", PDS_SLAB_ID_MAPPING,
                                  sizeof(mapping_entry),
                                  8192, true, true, NULL);
    SDK_ASSERT(mapping_slab_ != NULL);
}

mapping_state::~mapping_state() {
    ht::destroy(mapping_ht_);
    slab::destroy(mapping_slab_);
}

mapping_entry *
mapping_state::alloc(void) {
    return ((mapping_entry *)mapping_slab_->alloc());
}

sdk_ret_t
mapping_state::insert(mapping_entry *mapping) {
#if 0
    PDS_TRACE_VERBOSE("Inserting mapping - type %u, (%s, %s) to db",
                      mapping->key().type,
                      (mapping->key().type == PDS_MAPPING_TYPE_L2) ?
                          mapping->key().subnet.str() : mapping->key().vpc.str(),
                      (mapping->key().type == PDS_MAPPING_TYPE_L2) ?
                          macaddr2str(mapping->key().mac_addr) :
                          ipaddr2str(&mapping->key().ip_addr));
#endif
    if (mapping->key().type == PDS_MAPPING_TYPE_L2) {
        PDS_TRACE_VERBOSE("Inserting mapping - type %u, (%s, %s) to db",
                          mapping->key().type, mapping->key().subnet.str(),
                          macaddr2str(mapping->key().mac_addr));
    } else if (mapping->key().type == PDS_MAPPING_TYPE_L3) {
        PDS_TRACE_VERBOSE("Inserting mapping - type %u, (%s, %s) to db",
                          mapping->key().type,
                          std::string(mapping->key().vpc.str()),
                          ipaddr2str(&mapping->key().ip_addr));
    }
    return mapping_ht_->insert_with_key(&mapping->key_, mapping,
                                        &mapping->ht_ctxt_);
}

mapping_entry *
mapping_state::remove(mapping_entry *mapping) {
#if 0
    PDS_TRACE_VERBOSE("Removing mapping - type %u, (%s, %s) to db",
                      mapping->key().type,
                      (mapping->key().type == PDS_MAPPING_TYPE_L2) ?
                          mapping->key().subnet.str() : mapping->key().vpc.str(),
                      (mapping->key().type == PDS_MAPPING_TYPE_L2) ?
                          macaddr2str(mapping->key().mac_addr) :
                          ipaddr2str(&mapping->key().ip_addr));
#endif
    if (mapping->key().type == PDS_MAPPING_TYPE_L2) {
        PDS_TRACE_VERBOSE("Removing mapping - type %u, (%s, %s) to db",
                          mapping->key().type, mapping->key().subnet.str(),
                          macaddr2str(mapping->key().mac_addr));
    } else if (mapping->key().type == PDS_MAPPING_TYPE_L3) {
        PDS_TRACE_VERBOSE("Removing mapping - type %u, (%s, %s) to db",
                          mapping->key().type,
                          std::string(mapping->key().vpc.str()),
                          ipaddr2str(&mapping->key().ip_addr));
    }
    return (mapping_entry *)(mapping_ht_->remove(&mapping->key_));
}

void
mapping_state::free(mapping_entry *mapping) {
    mapping_slab_->free(mapping);
}

mapping_entry *
mapping_state::find(pds_mapping_key_t *key) const {
#if 0
    PDS_TRACE_VERBOSE("Looking for mapping - type %u, (%s, %s) to db",
                      key->type,
                      (key->type == PDS_MAPPING_TYPE_L2) ?
                          key->subnet.str() : key->vpc.str(),
                      (key->type == PDS_MAPPING_TYPE_L2) ?
                          macaddr2str(key->mac_addr) :
                          ipaddr2str(&key->ip_addr));
#endif
    if (key->type == PDS_MAPPING_TYPE_L2) {
        PDS_TRACE_VERBOSE("Looking for mapping - type %u, (%s, %s) to db",
                          key->type, key->subnet.str(),
                          macaddr2str(key->mac_addr));
    } else if (key->type == PDS_MAPPING_TYPE_L3) {
        PDS_TRACE_VERBOSE("Looking for mapping - type %u, (%s, %s) to db",
                          key->type, std::string(key->vpc.str()),
                          macaddr2str(key->mac_addr));
    }
    return (mapping_entry *)(mapping_ht_->lookup(key));
}

sdk_ret_t
mapping_state::walk(state_walk_cb_t walk_cb, void *ctxt) {
    return mapping_ht_->walk(walk_cb, ctxt);
}

sdk_ret_t
mapping_state::slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    walk_cb(mapping_slab_, ctxt);
    return SDK_RET_OK;
}

/** @} */    // end of PDS_MAPPING_STATE

}    // namespace api
