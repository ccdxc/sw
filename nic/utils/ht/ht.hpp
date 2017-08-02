//------------------------------------------------------------------------------
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
#ifndef __HT__
#define __HT__

#include <base.h>
#include <hal_lock.hpp>

namespace hal {
namespace utils {

// one class to put all hash functions together
class hash_algo {
public:
    // fnv hash function
    static uint32_t fnv_hash(void *key, uint32_t keylen) {
        uint8_t  *ptr = (uint8_t *)key;
        uint32_t i, hv = 2166136261;

        for (i = 0; i < keylen; i++) {
            hv = (hv * 16777619) ^ ptr[i];
        }

        return hv;
    }

    // TODO: bring CRC hash as well
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

typedef struct ht_bucket_ {
    uint32_t          num_entries;    // no. of entries in the bucket
    hal_spinlock_t    slock_;         // per bucket lock for thread safety
    ht_ctxt_t         *ht_ctxt;       // pointer to first entry
} ht_bucket_t;

// callback functions to be provided by the user of this library
typedef void *(*ht_get_key_func_t)(void *entry);
typedef uint32_t (*ht_compute_hash_func_t)(void *key, uint32_t ht_size);
typedef bool (*ht_compare_key_func_t)(void *key1, void *key2);

class ht {
public:
    static ht *factory(uint32_t ht_size, ht_get_key_func_t key_func,
                       ht_compute_hash_func_t hash_func,
                       ht_compare_key_func_t compare_func,
                       bool thread_safe=true);
    ~ht();

    // lookup() will return entry given its key or NULL if entry is not found
    void *lookup(void *key);

    hal_ret_t insert(void *entry, ht_ctxt_t *ht_ctxt);

    // remove API will remove entry with given key, if exists, from hash table
    // and return the entry or NULL if entry is not found
    void *remove(void *key);

    // remove_entry() will remove entry from table given it's hash context and
    // return the entry
    hal_ret_t remove_entry(void *entry, ht_ctxt_t *ht_ctxt);

    uint32_t size(void) const { return num_buckets_; }
    bool is_thread_safe(void) const { return thread_safe_; }
    uint32_t num_entries(void) const { return num_entries_; }
    uint32_t num_inserts(void) const { return num_inserts_; }
    uint32_t num_insert_errors(void) const { return num_insert_err_; }
    uint32_t num_removals(void) const { return num_removals_; }
    uint32_t num_removal_errors(void) const { return num_removal_err_; }
    uint32_t num_lookups(void) const { return num_lookups_; }

private:
    uint32_t                  num_buckets_;          // number of buckets
    ht_bucket_t               *ht_buckets_;          // actual hash table buckets
    bool                      thread_safe_;          // TRUE for thread safety
    hal_spinlock_t            slock_;                // lock for thread safety
    ht_compute_hash_func_t    hash_func_;            // hash function
    ht_get_key_func_t         get_key_func_;         // get key function
    ht_compare_key_func_t     compare_key_func_;     // key comparison function
    uint32_t                  num_entries_;          // total no. of entries in the table
    uint32_t                  num_inserts_;          // no. of insert operations so far
    uint32_t                  num_insert_err_;       // no. of insert errors
    uint32_t                  num_removals_;         // no. of remove operations so far
    uint32_t                  num_removal_err_;      // no. of remove errors
    uint32_t                  num_lookups_;          // no. of lookup operations so far
    uint32_t                  num_collisions_;       // no. of collisions seen so far

private:
    ht() { }
    bool init(uint32_t ht_size, ht_get_key_func_t key_func,
              ht_compute_hash_func_t hash_func,
              ht_compare_key_func_t compare_func,
              bool thread_safe);
    void *lookup_(ht_bucket_t *ht_bucket, void *key);
};

}    // namespace utils
}    // namespace hal

#endif    // __HT__
