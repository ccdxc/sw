//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// mapping database maintenance
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/mapping_state.hpp"

// TODO:
// 1. move PDS_TRACE_DEBUG to PDS_TRACE_VERBOSE

namespace api {

/// \defgroup PDS_MAPPING_STATE - mapping database functionality
/// \ingroup PDS_MAPPING
/// @{

// mapping hash table size in s/w
// NOTE: this has no relation to max# of mappings, it should be big enough to
// accomodate all mapping API calls in  a batch efficiently and because mappings
// are stateless objects, this hash table will be empty once a API batch is
// processed
#define PDS_MAPPING_HT_SIZE    (128 << 10)

mapping_state::mapping_state(sdk::lib::kvstore *kvs) {
    kvstore_ = kvs;
    mapping_ht_ = ht::factory(PDS_MAPPING_HT_SIZE,
                              mapping_entry::mapping_key_func_get,
                              sizeof(pds_obj_key_t));
    SDK_ASSERT(mapping_ht_ != NULL);
    mapping_skey_ht_ = ht::factory(PDS_MAPPING_HT_SIZE,
                                   mapping_entry::mapping_skey_func_get,
                                   sizeof(pds_mapping_key_t));
    SDK_ASSERT(mapping_skey_ht_ != NULL);
    mapping_slab_ = slab::factory("mapping", PDS_SLAB_ID_MAPPING,
                                  sizeof(mapping_entry),
                                  8192, true, true, NULL);
    SDK_ASSERT(mapping_slab_ != NULL);
}

mapping_state::~mapping_state() {
    ht::destroy(mapping_ht_);
    ht::destroy(mapping_skey_ht_);
    slab::destroy(mapping_slab_);
}

mapping_entry *
mapping_state::alloc(void) {
    return ((mapping_entry *)mapping_slab_->alloc());
}

sdk_ret_t
mapping_state::insert(mapping_entry *mapping) {
    sdk_ret_t ret;

    PDS_TRACE_DEBUG("Inserting %s", mapping->key2str().c_str());
    ret = mapping_skey_ht_->insert_with_key(&mapping->skey_, mapping,
                                            &mapping->skey_ht_ctxt_);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to insert mapping %s",
                      mapping->key2str().c_str());
        return ret;
    }
    if (mapping->key().valid()) {
        ret = mapping_ht_->insert_with_key(&mapping->key_, mapping,
                                           &mapping->ht_ctxt_);
    }
    return ret;
}

mapping_entry *
mapping_state::remove(mapping_entry *mapping) {
    void *ret;

    PDS_TRACE_DEBUG("Removing %s", mapping->key2str().c_str());
    ret = mapping_skey_ht_->remove(&mapping->skey_);
    if ((ret != NULL) && mapping->key().valid()) {
        return (mapping_entry *)(mapping_ht_->remove(&mapping->key_));
    }
    return (mapping_entry *)ret;
}

void
mapping_state::free(mapping_entry *mapping) {
    mapping_slab_->free(mapping);
}

mapping_entry *
mapping_state::find(pds_obj_key_t *key) const {
    PDS_TRACE_DEBUG("Looking for %s", key->str());
    return (mapping_entry *)(mapping_ht_->lookup(key));
}

// TODO: where are we using this ?
mapping_entry *
mapping_state::find(pds_mapping_key_t *skey) const {
    if (skey->type == PDS_MAPPING_TYPE_L2) {
        PDS_TRACE_DEBUG("Looking for mapping (%s, %s) to db",
                        skey->subnet.str(), macaddr2str(skey->mac_addr));
    } else if (skey->type == PDS_MAPPING_TYPE_L3) {
        PDS_TRACE_DEBUG("Looking for mapping (%s, %s) to db",
                        std::string(skey->vpc.str()),
                        ipaddr2str(&skey->ip_addr));
    }
    return (mapping_entry *)(mapping_skey_ht_->lookup(skey));
}

sdk_ret_t
mapping_state::skey(pds_obj_key_t *key, pds_mapping_key_t *skey) const {
    sdk_ret_t ret;
    size_t skey_sz = sizeof(pds_mapping_key_t);

    // find the 2nd-ary key from primary key
    ret = kvstore_->find(key, sizeof(*key), skey, &skey_sz);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("Primary key %s to 2nd-ary key lookup failed",
                        key->str());
    }
    return ret;
}

sdk_ret_t
mapping_state::persist(mapping_entry *mapping) {
    sdk_ret_t ret;

    // TODO: should we check if pkey is valid here ? we may have to support only
    //       skey based case also for internal use cases
    ret = kvstore_->insert(&mapping->key_, sizeof(mapping->key_),
                           &mapping->skey_, sizeof(mapping->skey_));
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to insert pkey -> skey binding in kvstore for"
                      "mapping %s, err %u", mapping->key2str().c_str(), ret);
    }
    return ret;
}

sdk_ret_t
mapping_state::perish(mapping_entry *mapping) {
    sdk_ret_t ret;

    // TODO: should we check if pkey is valid here ? we may have to support only
    //       skey based case also for internal use cases
    ret = kvstore_->remove(&mapping->key_, sizeof(mapping->key_));
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to remove pkey -> skey binding in kvstore for"
                      "mapping %s, err %u", mapping->key2str().c_str(), ret);
    }
    return ret;
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

/// \@}

}    // namespace api
