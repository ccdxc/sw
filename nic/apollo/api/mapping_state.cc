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
                                  8192, true, true, true, NULL);
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

    PDS_TRACE_VERBOSE("Inserting %s", mapping->key2str().c_str());
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
    void *rv;

    PDS_TRACE_VERBOSE("Removing %s", mapping->key2str().c_str());
    rv = mapping_skey_ht_->remove(&mapping->skey_);
    if ((rv != NULL) && mapping->key().valid()) {
        return (mapping_entry *)(mapping_ht_->remove(&mapping->key_));
    }
    return (mapping_entry *)rv;
}

void
mapping_state::free(mapping_entry *mapping) {
    mapping_slab_->free(mapping);
}

typedef struct pds_mapping_state_iterate_args_s {
    void *ctxt;
    mapping_state_cb_t cb;
} pds_mapping_state_iterate_args_t;

static void
mapping_state_entry_cb_ (void *key, void *val, void *ctxt) {
    mapping_entry *entry;
    pds_mapping_state_iterate_args_t *it_ctxt;
    pds_obj_key_t *pkey = (pds_obj_key_t *)key;

    it_ctxt = (pds_mapping_state_iterate_args_t *)ctxt;
    entry = mapping_entry::build(pkey);
    if (entry) {
        it_ctxt->cb(entry, it_ctxt->ctxt);
    }
}

void
mapping_state::kvstore_iterate(mapping_state_cb_t cb, void *ctxt) {
    pds_mapping_state_iterate_args_t it_ctxt;

    it_ctxt.cb = cb;
    it_ctxt.ctxt = ctxt;
    kvstore_->iterate(mapping_state_entry_cb_, &it_ctxt);
}

mapping_entry *
mapping_state::find(pds_obj_key_t *key) const {
    PDS_TRACE_VERBOSE("Looking for %s", key->str());
    return (mapping_entry *)(mapping_ht_->lookup(key));
}

mapping_entry *
mapping_state::find(pds_mapping_key_t *skey) const {
    if (skey->type == PDS_MAPPING_TYPE_L2) {
        PDS_TRACE_VERBOSE("Looking for mapping (%s, %s) to db",
                          skey->subnet.str(), macaddr2str(skey->mac_addr));
    } else if (skey->type == PDS_MAPPING_TYPE_L3) {
        PDS_TRACE_VERBOSE("Looking for mapping (%s, %s) to db",
                          skey->vpc.str(), ipaddr2str(&skey->ip_addr));
    } else {
        SDK_ASSERT(FALSE);
        return NULL;
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
        PDS_TRACE_VERBOSE("Primary key %s to 2nd-ary key lookup failed "
                          "for mapping", key->str());
    }
    return ret;
}

sdk_ret_t
mapping_state::persist(mapping_entry *mapping, pds_mapping_spec_t *spec) {
    sdk_ret_t ret;

    if (mapping->key_.valid()) {
        ret = kvstore_->insert(&mapping->key_, sizeof(mapping->key_),
                               &mapping->skey_, sizeof(mapping->skey_));
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to insert pkey -> skey binding in kvstore for"
                          "mapping %s, err %u", mapping->key2str().c_str(),
                          ret);
        }
        return ret;
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_state::perish(const pds_obj_key_t& key) {
    sdk_ret_t ret;

    if (key.valid()) {
        ret = kvstore_->remove(&key, sizeof(key));
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to remove pkey -> skey binding in kvstore for"
                          "mapping %s, err %u", key.str(), ret);
        }
        return ret;
    }
    return SDK_RET_OK;
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
