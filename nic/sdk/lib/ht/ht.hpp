//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
//  intrusive (aka. nosy) hash table library
//  To use this lib, the hash table entry allocated by the app should look like
//  below (order of the fields is not important):
//
//  typedef struct/class app_entry_s {
//      ...
//      ...
//      struct key_s {
//          field1;
//          field2;
//      } key;
//      ...
//      ...
//      ht_ctxt_t ht_ctxt;
//      ...
//      ...
//  } app_entry_t;
//
//      OR
//
//  typedef struct/class app_entry_s {
//      ...
//      ...
//      key_t *key;
//      ...
//      ht_ctxt_t ht_ctxt;
//      ...
//      ...
//  } app_entry_t;
//
//  This way user allocates the whole entry in one shot and key, data and all
//  context that hash table maintains is all in one place. A slab can be used
//  to allocate such user entry. An intrusive data structure will result in less
//  memory fragmentation
//------------------------------------------------------------------------------

#ifndef __SDK_HT_HPP__
#define __SDK_HT_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/lock.hpp"
#include "lib/shmmgr/shmmgr.hpp"

namespace sdk {
namespace lib {

// one class to put all hash (static) functions together
class hash_algo {
public:
    // fnv hash function
    static uint32_t fnv_hash(void *key, uint32_t keylen) {
        uint8_t     *ptr = (uint8_t *)key;
        uint32_t    i, hv = 2166136261;

        for (i = 0; i < keylen; i++) {
            hv = (hv * 16777619) ^ ptr[i];
        }

        return hv;
    }
};

typedef struct ht_ctxt_s ht_ctxt_t;
struct ht_ctxt_s {
    void         *entry;    // pointer to entry that has this context
    ht_ctxt_t    *prev;     // prev link
    ht_ctxt_t    *next;     // next link

    void reset(void) {
        entry = NULL;
        prev = next = NULL;
    }
} __PACK__;

class ht {
public:
    // callback functions to be provided by the user of this library
    typedef void *(*ht_get_key_func_t)(void *entry);
    typedef bool (ht_walk_cb_t)(void *entry, void *ctxt);

    static ht *factory(uint32_t ht_size, ht_get_key_func_t key_func,
                       uint32_t key_size, bool thread_safe=true,
                       bool key_string=false, shmmgr *mmgr=NULL);
    static void destroy(ht *htable, shmmgr *mmgr=NULL);

    // lookup() will return entry given its key or NULL if entry is not found
    void *lookup(void *key);

    // insert APIs
    sdk_ret_t insert_with_key(void *key, void *entry, ht_ctxt_t *ht_ctxt);
    sdk_ret_t insert(void *entry, ht_ctxt_t *ht_ctxt);

    // remove API will remove entry with given key, if exists, from hash table
    // and return the entry or NULL if entry is not found
    void *remove(void *key);

    // remove_entry() will remove entry from table given it's hash context and
    // return the entry
    sdk_ret_t remove_entry(void *entry, ht_ctxt_t *ht_ctxt);

    // walk the whole hash table (without taking any locks, not thread safe if
    // ht instance is shared), if the callback function returns true, walk is
    // stopped
    sdk_ret_t walk(ht_walk_cb_t walk_cb, void *ctxt);

    // walk the whole hash table in thread safe manner (i.e., locking at each
    // bucket level), if the callback function returns true, walk is stopped
    sdk_ret_t walk_safe(ht_walk_cb_t walk_cb, void *ctxt);

    // walk given bucket in thread safe manner (i.e., locking the bucket),
    // if the callback function returns true, walk is stopped .. if the bucket
    // given is out of range, walk will fail
    sdk_ret_t walk_bucket_safe(uint64_t bucket, ht_walk_cb_t walk_cb, void *ctxt);

    // helpers for debugging
    uint32_t size(void) const { return num_buckets_; }
    bool is_thread_safe(void) const { return thread_safe_; }
    uint32_t num_buckets(void) const { return num_buckets_; }
    uint32_t num_entries(void) const { return num_entries_; }
    uint32_t num_inserts(void) const { return num_inserts_; }
    uint32_t num_insert_errors(void) const { return num_insert_err_; }
    uint32_t num_removals(void) const { return num_removals_; }
    uint32_t num_removal_errors(void) const { return num_removal_err_; }
    uint32_t num_lookups(void) const { return num_lookups_; }

private:
    typedef struct ht_bucket_ {
        uint32_t          num_entries;    // no. of entries in the bucket
        sdk_spinlock_t    slock_;         // per bucket lock for thread safety
        ht_ctxt_t         *ht_ctxt;       // pointer to first entry
    } ht_bucket_t;

    uint32_t              num_buckets_;          // number of buckets
    ht_bucket_t           *ht_buckets_;          // actual hash table buckets
    bool                  thread_safe_;          // TRUE for thread safety
    bool                  key_string_;           // TRUE if key is a string
    sdk_spinlock_t        slock_;                // lock for thread safety
    ht_get_key_func_t     get_key_func_;         // get key function
    uint32_t              key_size_;             // key size
    uint32_t              num_entries_;          // total no. of entries in the table
    uint32_t              num_inserts_;          // no. of insert operations so far
    uint32_t              num_insert_err_;       // no. of insert errors
    uint32_t              num_removals_;         // no. of remove operations so far
    uint32_t              num_removal_err_;      // no. of remove errors
    uint32_t              num_lookups_;          // no. of lookup operations so far

private:
    ht() { }
    ~ht();
    bool init(uint32_t ht_size, ht_get_key_func_t key_func,
              uint32_t key_size, bool thread_safe,
              bool key_string, shmmgr *mmgr);
    static void cleanup(ht *htable, shmmgr *mmgr);
    void *lookup_(ht_bucket_t *ht_bucket, void *key);
};

}    // namespace lib
}    // namespace sdk

using sdk::lib::ht;
using sdk::lib::ht_ctxt_t;
using sdk::lib::hash_algo;

#if 0
#define SDK_HT_CREATE(name, htable, table_sz, ...)                             \
do {                                                                           \
    SDK_TRACE_DEBUG("Creating %s hash table with size %u", (name), (table_sz));\
    (htable) = ht::factory((table_sz), __VA_ARGS__);                           \
} while (0)
#endif

#endif    // __SDK_HT_HPP__

