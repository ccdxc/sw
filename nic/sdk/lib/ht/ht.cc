//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "include/sdk/mem.hpp"
#include "lib/ht/ht.hpp"
#include <cstring>

namespace sdk {
namespace lib {

// common initialization
bool
ht::init(uint32_t ht_size, ht_get_key_func_t get_key_func,
         uint32_t key_size, bool thread_safe, bool key_string,
         shmmgr *mmgr)
{
    uint32_t    i;

    num_buckets_ = ht_size;
    thread_safe_ = thread_safe;
    get_key_func_ = get_key_func;
    key_size_ = key_size;
    key_string_ = key_string;
    if (thread_safe) {
        if (mmgr) {
            SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_SHARED);
        } else {
            SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
        }
    }

    num_entries_ = 0;
    num_inserts_ = 0;
    num_insert_err_ = 0;
    num_removals_ = 0;
    num_removal_err_ = 0;
    num_lookups_ = 0;

    for (i = 0; i < ht_size; i++) {
        if (mmgr) {
            SDK_SPINLOCK_INIT(&ht_buckets_[i].slock_, PTHREAD_PROCESS_SHARED);
        } else {
            SDK_SPINLOCK_INIT(&ht_buckets_[i].slock_, PTHREAD_PROCESS_PRIVATE);
        }
    }
    return true;
}

void
ht::cleanup(ht *htable, shmmgr *mmgr)
{
    if (mmgr) {
        if (htable->ht_buckets_) {
            mmgr->free(htable->ht_buckets_);
        }
        htable->~ht();
        mmgr->free(htable);
    } else {
        if (htable->ht_buckets_) {
            SDK_FREE(SDK_MEM_ALLOC_LIB_HT,
                     htable->ht_buckets_);
            htable->~ht();
            SDK_FREE(SDK_MEM_ALLOC_LIB_HT, htable);
        }
    }
}

// factory method for the hash table
ht *
ht::factory(uint32_t ht_size, ht_get_key_func_t get_key_func,
            uint32_t key_size, bool thread_safe, bool key_string,
            shmmgr *mmgr)
{
    void    *mem;
    ht      *hash_table = NULL;

    SDK_ASSERT_RETURN((get_key_func != NULL) && 
                      (ht_size > 0) && (key_size > 0),
                      NULL);

    if (mmgr) {
        mem = mmgr->alloc(sizeof(ht), 4, true);
    } else {
        mem = SDK_CALLOC(SDK_MEM_ALLOC_LIB_HT, sizeof(ht));
    }
    if (!mem) {
        return NULL;
    }
    hash_table = new (mem) ht();

    // TODO: buckets should be a function of ht_size
    if (mmgr) {
        hash_table->ht_buckets_ =
            (ht_bucket_t *)mmgr->alloc(ht_size * sizeof(ht_bucket_t),
                                       4, true);
    } else {
        hash_table->ht_buckets_ =
            (ht_bucket_t *)SDK_CALLOC(SDK_MEM_ALLOC_LIB_HT,
                                      ht_size * sizeof(ht_bucket_t));
    }
    if (hash_table->ht_buckets_ == NULL) {
        goto cleanup;
    }
    // SDK_TRACE_DEBUG("Allocated %u bytes for ht", ht_size * sizeof(ht_bucket_t));

    if (hash_table->init(ht_size, get_key_func,
                         key_size, thread_safe,
                         key_string, mmgr) == false) {
        goto cleanup;
    }
    return hash_table;

cleanup:

    if (hash_table) {
        ht::cleanup(hash_table, mmgr);
    }
    return NULL;
}

ht::~ht()
{
    if (thread_safe_) {
        SDK_SPINLOCK_DESTROY(&slock_);
    }
}

void
ht::destroy(ht *htable, shmmgr *mmgr)
{
    if (!htable) {
        return;
    }
    ht::cleanup(htable, mmgr);
}

// internal helper function that looks up an entry given its key and the bucket
// to search in
void *
ht::lookup_(ht_bucket_t *ht_bucket, void *key)
{
    ht_ctxt_t    *ht_ctxt;
    bool         match;
    void         *entry_key;

    ht_ctxt = ht_bucket->ht_ctxt;
    while (ht_ctxt) {
        entry_key = get_key_func_(ht_ctxt->entry);
        if (key_string_) {
            match = (strcmp((char *)key, (char *)entry_key) == 0);
        } else {
            match = (std::memcmp(key, entry_key, key_size_) == 0);
        }
        if (match == true) {
            // found what we are looking for
            return ht_ctxt->entry;
        }
        ht_ctxt = ht_ctxt->next;
    }
    return NULL;
}

void *
ht::lookup(void *key)
{
    uint32_t       hash_val;
    ht_bucket_t    *ht_bucket;
    void           *entry = NULL;

    SDK_ASSERT_RETURN(key != NULL, NULL);
    if (key_string_) {
        hash_val = hash_algo::fnv_hash(key, strlen((char *)key)) % num_buckets_;
    } else {
        hash_val = hash_algo::fnv_hash(key, key_size_) % num_buckets_;
    }
    SDK_ASSERT_RETURN((hash_val < num_buckets_), NULL);

    SDK_ATOMIC_INC_UINT32(&this->num_lookups_, 1);
    ht_bucket = &this->ht_buckets_[hash_val];

    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_LOCK(&ht_bucket->slock_) == 0), NULL);
    }
    entry = lookup_(ht_bucket, key);
    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&ht_bucket->slock_) == 0),
                           NULL);
    }

    return entry;
}

sdk_ret_t
ht::insert_with_key(void *key, void *entry, ht_ctxt_t *ht_ctxt)
{
    void           *old_entry;
    uint32_t       hash_val;
    ht_bucket_t    *ht_bucket;

    SDK_ASSERT_RETURN(key != NULL, SDK_RET_ERR);
    if (key_string_) {
        hash_val = hash_algo::fnv_hash(key, strlen((char *)key)) % num_buckets_;
    } else {
        hash_val = hash_algo::fnv_hash(key, key_size_) % num_buckets_;
    }
    SDK_ASSERT_RETURN((hash_val < num_buckets_), SDK_RET_ERR);

    SDK_ATOMIC_INC_UINT32(&this->num_inserts_, 1);
    ht_bucket = &this->ht_buckets_[hash_val];
    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_LOCK(&ht_bucket->slock_) == 0),
                          SDK_RET_ERR);
    }
    old_entry = lookup_(ht_bucket, key);
    if (old_entry) {
        // an entry exists already with this key
        SDK_ATOMIC_INC_UINT32(&this->num_insert_err_, 1);
        if (thread_safe_) {
            SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&ht_bucket->slock_) == 0),
                               SDK_RET_ERR);
        }
        return SDK_RET_ENTRY_EXISTS;
    }
    if (ht_bucket->ht_ctxt) {
        ht_ctxt->next = ht_bucket->ht_ctxt;
        ht_bucket->ht_ctxt->prev = ht_ctxt;
    }
    ht_ctxt->entry = entry;
    ht_bucket->ht_ctxt = ht_ctxt;
    ht_bucket->num_entries++;
    SDK_ATOMIC_INC_UINT32(&num_entries_, 1);
    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&ht_bucket->slock_) == 0),
                           SDK_RET_ERR);
    }
    return SDK_RET_OK;
}

sdk_ret_t
ht::insert(void *entry, ht_ctxt_t *ht_ctxt)
{
    SDK_ASSERT_RETURN(((entry != NULL) && (ht_ctxt != NULL)),
                      SDK_RET_INVALID_ARG);
    SDK_ASSERT_RETURN(((ht_ctxt->prev == NULL) && (ht_ctxt->next == NULL)),
                      SDK_RET_INVALID_ARG);
    return insert_with_key(get_key_func_(entry), entry, ht_ctxt);
}

void *
ht::remove(void *key)
{
    uint32_t        hash_val;
    ht_bucket_t    *ht_bucket;
    void           *entry;
    ht_ctxt_t      *curr_ctxt, *prev_ctxt;
    bool           match = false;

    SDK_ASSERT_RETURN(key != NULL, NULL);
    if (key_string_) {
        hash_val = hash_algo::fnv_hash(key, strlen((char *)key)) % num_buckets_;
    } else {
        hash_val = hash_algo::fnv_hash(key, key_size_) % num_buckets_;
    }
    SDK_ASSERT_RETURN((hash_val < num_buckets_), NULL);

    SDK_ATOMIC_INC_UINT32(&this->num_removals_, 1);
    ht_bucket = &this->ht_buckets_[hash_val];
    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_LOCK(&ht_bucket->slock_) == 0), NULL);
    }
    prev_ctxt = curr_ctxt = ht_bucket->ht_ctxt;
    if (curr_ctxt == NULL) {
        // there are no entries in this bucket
        if (thread_safe_) {
            this->num_removal_err_++;
            SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&ht_bucket->slock_) == 0),
                               NULL);
        }
        return NULL;
    }

    do {
        if (key_string_) {
            match = (strcmp((char *)key, (char *)get_key_func_(curr_ctxt->entry)) == 0);
        } else {
            match = (std::memcmp(key, get_key_func_(curr_ctxt->entry), key_size_) == 0);
        }
        if (match == true) {
            // entry found, remove it
            if (curr_ctxt == ht_bucket->ht_ctxt) {
                // removing the first entry
                ht_bucket->ht_ctxt = curr_ctxt->next;
                if (ht_bucket->ht_ctxt) {
                    ht_bucket->ht_ctxt->prev = NULL;
                }
            } else if (curr_ctxt->next == NULL) {
                prev_ctxt->next = NULL;
            } else {
                prev_ctxt->next = curr_ctxt->next;
                curr_ctxt->next->prev = prev_ctxt;
            }
            entry = curr_ctxt->entry;
            curr_ctxt->reset();
            ht_bucket->num_entries--;
            if (thread_safe_) {
                SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&ht_bucket->slock_) == 0),
                                   NULL);
            }
            SDK_ATOMIC_DEC_UINT32(&num_entries_, 1);
            return entry;
        }
        prev_ctxt = curr_ctxt;
        curr_ctxt = curr_ctxt->next;
    } while (curr_ctxt != NULL);

    // entry to remove not found
    this->num_removal_err_++;
    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&ht_bucket->slock_) == 0),
                           NULL);
    }
    return NULL;
}

sdk_ret_t
ht::remove_entry(void *entry, ht_ctxt_t *ht_ctxt)
{
    uint32_t       hash_val;
    ht_bucket_t    *ht_bucket;
    void           *key;

    SDK_ASSERT_RETURN(((entry != NULL) && (ht_ctxt != NULL)),
                      SDK_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((entry == ht_ctxt->entry), SDK_RET_INVALID_ARG);

    key = get_key_func_(entry);
    SDK_ASSERT_RETURN(key != NULL, SDK_RET_ERR);
    if (key_string_) {
        hash_val = hash_algo::fnv_hash(key, strlen((char *)key)) % num_buckets_;
    } else {
        hash_val = hash_algo::fnv_hash(key, key_size_) % num_buckets_;
    }
    SDK_ASSERT_RETURN((hash_val < num_buckets_), SDK_RET_ERR);

    SDK_ATOMIC_INC_UINT32(&this->num_removals_, 1);
    ht_bucket = &this->ht_buckets_[hash_val];
    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_LOCK(&ht_bucket->slock_) == 0),
                          SDK_RET_ERR);
    }

    if (ht_bucket->ht_ctxt == ht_ctxt) {
        ht_bucket->ht_ctxt = ht_ctxt->next;
        if (ht_bucket->ht_ctxt) {
            ht_bucket->ht_ctxt->prev = NULL;
        }
    } else if (ht_ctxt->next == NULL) {
        ht_ctxt->prev->next = NULL;
    } else {
        ht_ctxt->prev->next = ht_ctxt->next;
        ht_ctxt->next->prev = ht_ctxt->prev;
    }
    ht_ctxt->reset();
    ht_bucket->num_entries--;
    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&ht_bucket->slock_) == 0),
                          SDK_RET_ERR);
    }
    SDK_ATOMIC_DEC_UINT32(&num_entries_, 1);
    return SDK_RET_OK;
}

sdk_ret_t
ht::walk(ht_walk_cb_t walk_cb, void *ctxt)
{
    uint32_t     i;
    ht_ctxt_t    *curr, *next;
    bool         stop_walk = false;

    SDK_ASSERT_RETURN((walk_cb != NULL), SDK_RET_INVALID_ARG);
    for (i = 0; i < num_buckets_; i++) {
        curr = ht_buckets_[i].ht_ctxt;
        while (curr) {
            // cache the next entry so it is delete-safe
            next = curr->next;
            stop_walk = walk_cb(curr->entry, ctxt);
            if (stop_walk) {
                goto end;
            }
            curr = next;
        }
    }

end:

    return SDK_RET_OK;
}

sdk_ret_t
ht::walk_safe(ht_walk_cb_t walk_cb, void *ctxt)
{
    uint32_t     i;
    ht_ctxt_t    *curr, *next;
    bool         stop_walk = false;

    SDK_ASSERT_RETURN((walk_cb != NULL), SDK_RET_INVALID_ARG);
    for (i = 0; i < num_buckets_; i++) {
        if (thread_safe_) {
            SDK_SPINLOCK_LOCK(&ht_buckets_[i].slock_);
        }
        curr = ht_buckets_[i].ht_ctxt;
        while (curr) {
            // cache the next entry so it is delete-safe
            next = curr->next;
            stop_walk = walk_cb(curr->entry, ctxt);
            if (stop_walk) {
                if (thread_safe_) {
                    SDK_SPINLOCK_UNLOCK(&ht_buckets_[i].slock_);
                }
                goto end;
            }
            curr = next;
        }
        if (thread_safe_) {
            SDK_SPINLOCK_UNLOCK(&ht_buckets_[i].slock_);
        }
    }

end:

    return SDK_RET_OK;
}

sdk_ret_t
ht::walk_bucket_safe(uint64_t bucket, ht_walk_cb_t walk_cb, void *ctxt)
{
    ht_ctxt_t    *curr, *next;
    bool         stop_walk = false;

    if (bucket >= num_buckets_) {
        return SDK_RET_INVALID_ARG;
    }
    curr = ht_buckets_[bucket].ht_ctxt;
    if (thread_safe_) {
        SDK_SPINLOCK_LOCK(&ht_buckets_[bucket].slock_);
    }
    while (curr) {
        // cache the next entry so it is delete-safe
        next = curr->next;
        stop_walk = walk_cb(curr->entry, ctxt);
        if (stop_walk) {
            goto end;
        }
        curr = next;
    }

end:

    if (thread_safe_) {
        SDK_SPINLOCK_UNLOCK(&ht_buckets_[bucket].slock_);
    }
    return SDK_RET_OK;
}

}    // namespace lib
}    // namespace sdk
