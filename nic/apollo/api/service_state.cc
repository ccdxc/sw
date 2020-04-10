//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// service mapping databse handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/service_state.hpp"

namespace api {

/// \defgroup PDS_SVC_MAPPING_STATE - mservice apping database functionality
/// \ingroup PDS_SVC_MAPPING
/// \@{

// service mapping hash table size in s/w
// NOTE: this has no relation to max# of service mappings, it should be big
//       enough to accomodate all svc mapping API calls in a batch efficiently
//       and because service mappings are stateless objects, this hash table
//       will be empty once a API batch is processed
#define PDS_SVC_MAPPING_HT_SIZE    1024

svc_mapping_state::svc_mapping_state(sdk::lib::kvstore *kvs) {
    kvstore_ = kvs;
    svc_mapping_ht_ = ht::factory(PDS_SVC_MAPPING_HT_SIZE,
                                  svc_mapping::svc_mapping_key_func_get,
                                  sizeof(pds_obj_key_t));
    SDK_ASSERT(svc_mapping_ht_ != NULL);
    svc_mapping_skey_ht_ = ht::factory(PDS_SVC_MAPPING_HT_SIZE,
                                       svc_mapping::svc_mapping_skey_func_get,
                                       sizeof(pds_svc_mapping_key_t));
    SDK_ASSERT(svc_mapping_skey_ht_ != NULL);
    svc_mapping_slab_ = slab::factory("svc-mapping", PDS_SLAB_ID_SVC_MAPPING,
                                      sizeof(svc_mapping), 64, true, true,
                                      true, NULL);
    SDK_ASSERT(svc_mapping_slab_ != NULL);
}

svc_mapping_state::~svc_mapping_state() {
    ht::destroy(svc_mapping_ht_);
    ht::destroy(svc_mapping_skey_ht_);
    slab::destroy(svc_mapping_slab_);
}

svc_mapping *
svc_mapping_state::alloc(void) {
    return ((svc_mapping *)svc_mapping_slab_->alloc());
}

sdk_ret_t
svc_mapping_state::insert(svc_mapping *mapping) {
    sdk_ret_t ret;

    ret = svc_mapping_skey_ht_->insert_with_key(&mapping->skey_, mapping,
                                                &mapping->skey_ht_ctxt_);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to insert svc mapping %s",
                      mapping->key2str().c_str());
        return ret;
    }

    if (mapping->key().valid()) {
        return svc_mapping_ht_->insert_with_key(&mapping->key_, mapping,
                                                &mapping->ht_ctxt_);
    }
    return ret;
}

svc_mapping *
svc_mapping_state::remove(svc_mapping *mapping) {
    void *rv;

    rv = svc_mapping_skey_ht_->remove(&mapping->skey_);
    if (rv && mapping->key().valid()) {
        return (svc_mapping *)(svc_mapping_ht_->remove(&mapping->key_));
    }
    return (svc_mapping *)rv;
}

void
svc_mapping_state::free(svc_mapping *mapping) {
    svc_mapping_slab_->free(mapping);
}

typedef struct pds_svc_mapping_state_iterate_args_s {
    void *ctxt;
    svc_mapping_state_cb_t cb;
} pds_svc_mapping_state_iterate_args_t;

static void
svc_mapping_state_entry_cb_ (void *key, void *val, void *ctxt) {
    svc_mapping *entry;
    pds_svc_mapping_state_iterate_args_t *it_ctxt;
    pds_obj_key_t *pkey = (pds_obj_key_t *)key;

    it_ctxt = (pds_svc_mapping_state_iterate_args_t *)ctxt;
    entry = svc_mapping::build(pkey);
    if (entry) {
        it_ctxt->cb(entry, it_ctxt->ctxt);
    }
}

void
svc_mapping_state::kvstore_iterate(svc_mapping_state_cb_t cb, void *ctxt) {
    pds_svc_mapping_state_iterate_args_t it_ctxt;

    it_ctxt.cb = cb;
    it_ctxt.ctxt = ctxt;
    kvstore_->iterate(svc_mapping_state_entry_cb_, &it_ctxt, "svc");
}

svc_mapping *
svc_mapping_state::find(pds_obj_key_t *key) const {
    PDS_TRACE_VERBOSE("Looking for %s", key->str());
    return (svc_mapping *)(svc_mapping_ht_->lookup(key));
}

svc_mapping *
svc_mapping_state::find(pds_svc_mapping_key_t *skey) const {
    return (svc_mapping *)(svc_mapping_skey_ht_->lookup(skey));
}

sdk_ret_t
svc_mapping_state::skey(pds_obj_key_t *key, pds_svc_mapping_key_t *skey) const {
    sdk_ret_t ret;
    size_t skey_sz = sizeof(pds_svc_mapping_key_t);

    // find the 2nd-ary key from primary key
    ret = kvstore_->find(key, sizeof(*key), skey, &skey_sz, "svc");
    if (ret != SDK_RET_OK) {
        PDS_TRACE_VERBOSE("Primary key %s to 2nd-ary key lookup failed "
                          "for svc mapping", key->str());
    }
    return ret;
}

sdk_ret_t
svc_mapping_state::persist(svc_mapping *mapping,
                           pds_svc_mapping_spec_t *spec) {
    sdk_ret_t ret;

    if (mapping->key_.valid()) {
        ret = kvstore_->insert(&mapping->key_, sizeof(mapping->key_),
                               &mapping->skey_, sizeof(mapping->skey_), "svc");
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to insert pkey -> skey binding in kvstore for"
                          "svc mapping %s, err %u", mapping->key2str().c_str(),
                          ret);
        }
        return ret;
    }
    return SDK_RET_OK;
}

sdk_ret_t
svc_mapping_state::perish(const pds_obj_key_t& key) {
    sdk_ret_t ret;

    if (key.valid()) {
        ret = kvstore_->remove(&key, sizeof(key), "svc");
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to remove pkey -> skey binding in kvstore for"
                          "svc mapping %s, err %u", key.str(), ret);
        }
        return ret;
    }
    return SDK_RET_OK;
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

/// \@}

}    // namespace api
