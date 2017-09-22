#include <assert.h>
#include <hal_mem.hpp>
#include <ht.hpp>

namespace hal {
namespace utils {

// common initialization
bool
ht::init(uint32_t ht_size, ht_get_key_func_t get_key_func,
        ht_compute_hash_func_t hash_func, ht_compare_key_func_t compare_func,
        bool thread_safe)
{
    uint32_t    i;

    num_buckets_ = ht_size;
    ht_buckets_ = NULL;
    thread_safe_ = thread_safe;
    hash_func_ = hash_func;
    get_key_func_ = get_key_func;
    compare_key_func_ = compare_func;
    if (thread_safe) {
        HAL_ASSERT_RETURN(!HAL_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE),
                          false);
    }
                           
    num_entries_ = 0;
    num_inserts_ = 0;
    num_insert_err_ = 0;
    num_removals_ = 0;
    num_removal_err_ = 0;
    num_lookups_ = 0;
    num_collisions_ = 0;
    // TODO: buckets should be a function of ht_size
    ht_buckets_ = (ht_bucket_t *)HAL_MALLOCZ(HAL_MEM_ALLOC_LIB_HT,
                                             ht_size * sizeof(ht_bucket_t));
    if (ht_buckets_ == NULL) {
        return false;
    }
    for (i = 0; i < ht_size; i++) {
        HAL_SPINLOCK_INIT(&ht_buckets_[i].slock_, PTHREAD_PROCESS_PRIVATE);
    }
    return true;
}

// factory method for the hash table
ht *
ht::factory(uint32_t ht_size, ht_get_key_func_t get_key_func,
            ht_compute_hash_func_t hash_func,
            ht_compare_key_func_t compare_func, bool thread_safe)
{
    ht    *hash_table;

    HAL_ASSERT_RETURN((ht_size > 0), NULL);
    HAL_ASSERT_RETURN(((get_key_func != NULL) && (hash_func != NULL) &&
                       (compare_func != NULL)), NULL);

    hash_table = new ht();
    if (hash_table == NULL) {
        return NULL;
    }
    if (hash_table->init(ht_size, get_key_func, hash_func,
                         compare_func, thread_safe) == false) {
        delete hash_table;
        return NULL;
    }
    return hash_table;
}

ht::~ht()
{
    if (ht_buckets_) {
        delete ht_buckets_;
    }
    if (thread_safe_) {
        HAL_SPINLOCK_DESTROY(&slock_);
    }
}

// internal helper function that looks up an entry given its key and the bucket
// to search in
void *
ht::lookup_(ht_bucket_t *ht_bucket, void *key)
{
    ht_ctxt_t    *ht_ctxt;
    bool         match;

    ht_ctxt = ht_bucket->ht_ctxt;
    while (ht_ctxt) {
        match = compare_key_func_(key, get_key_func_(ht_ctxt->entry));
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

    HAL_ASSERT_RETURN(key != NULL, NULL);
    hash_val = hash_func_(key, num_buckets_);
    HAL_ASSERT_RETURN((hash_val < num_buckets_), NULL);

    HAL_ATOMIC_INC_UINT32(&this->num_lookups_, 1);
    ht_bucket = &this->ht_buckets_[hash_val];

    if (thread_safe_) {
        HAL_ASSERT_RETURN((HAL_SPINLOCK_LOCK(&ht_bucket->slock_) == 0), NULL);
    }
    entry = lookup_(ht_bucket, key);
    if (thread_safe_) {
        HAL_ASSERT_RETURN((HAL_SPINLOCK_UNLOCK(&ht_bucket->slock_) == 0),
                           NULL);
    }

    return entry;
}

hal_ret_t
ht::insert_with_key(void *key, void *entry, ht_ctxt_t *ht_ctxt)
{
    void           *old_entry;
    uint32_t       hash_val;
    ht_bucket_t    *ht_bucket;

    HAL_ASSERT_RETURN(key != NULL, HAL_RET_ERR);
    hash_val = hash_func_(key, num_buckets_);
    HAL_ASSERT_RETURN((hash_val < num_buckets_), HAL_RET_ERR);

    HAL_ATOMIC_INC_UINT32(&this->num_inserts_, 1);
    ht_bucket = &this->ht_buckets_[hash_val];
    if (thread_safe_) {
        HAL_ASSERT_RETURN((HAL_SPINLOCK_LOCK(&ht_bucket->slock_) == 0),
                          HAL_RET_ERR);
    }
    old_entry = lookup_(ht_bucket, key);
    if (old_entry) {
        // an entry exists already with this key
        HAL_ATOMIC_INC_UINT32(&this->num_insert_err_, 1);
        if (thread_safe_) {
            HAL_ASSERT_RETURN((HAL_SPINLOCK_UNLOCK(&ht_bucket->slock_) == 0),
                               HAL_RET_ERR);
        }
        return HAL_RET_ENTRY_EXISTS;
    }
    if (ht_bucket->ht_ctxt) {
        ht_ctxt->next = ht_bucket->ht_ctxt;
        ht_bucket->ht_ctxt->prev = ht_ctxt;
    }
    ht_ctxt->entry = entry;
    ht_bucket->ht_ctxt = ht_ctxt;
    ht_bucket->num_entries++;
    HAL_ATOMIC_INC_UINT32(&num_entries_, 1);
    if (thread_safe_) {
        HAL_ASSERT_RETURN((HAL_SPINLOCK_UNLOCK(&ht_bucket->slock_) == 0),
                           HAL_RET_ERR);
    }
    return HAL_RET_OK;
}

hal_ret_t
ht::insert(void *entry, ht_ctxt_t *ht_ctxt)
{
    HAL_ASSERT_RETURN(((entry != NULL) && (ht_ctxt != NULL)),
                      HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN(((ht_ctxt->prev == NULL) && (ht_ctxt->next == NULL)),
                      HAL_RET_INVALID_ARG);
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

    HAL_ASSERT_RETURN(key != NULL, NULL);
    hash_val = hash_func_(key, num_buckets_);
    HAL_ASSERT_RETURN((hash_val < num_buckets_), NULL);

    HAL_ATOMIC_INC_UINT32(&this->num_removals_, 1);
    ht_bucket = &this->ht_buckets_[hash_val];
    if (thread_safe_) {
        HAL_ASSERT_RETURN((HAL_SPINLOCK_LOCK(&ht_bucket->slock_) == 0), NULL);
    }
    prev_ctxt = curr_ctxt = ht_bucket->ht_ctxt;
    if (curr_ctxt == NULL) {
        // there are no entries in this bucket
        if (thread_safe_) {
            this->num_removal_err_++;
            HAL_ASSERT_RETURN((HAL_SPINLOCK_UNLOCK(&ht_bucket->slock_) == 0),
                               NULL);
        }
        return NULL;
    }

    do {
        match = compare_key_func_(key, get_key_func_(curr_ctxt->entry));
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
                HAL_ASSERT_RETURN((HAL_SPINLOCK_UNLOCK(&ht_bucket->slock_) == 0),
                                   NULL);
            }
            HAL_ATOMIC_DEC_UINT32(&num_entries_, 1);
            return entry;
        }
        prev_ctxt = curr_ctxt;
        curr_ctxt = curr_ctxt->next;
    } while (curr_ctxt != NULL);

    // entry to remove not found
    this->num_removal_err_++;
    if (thread_safe_) {
        HAL_ASSERT_RETURN((HAL_SPINLOCK_UNLOCK(&ht_bucket->slock_) == 0),
                           NULL);
    }
    return NULL;
}

hal_ret_t
ht::remove_entry(void *entry, ht_ctxt_t *ht_ctxt)
{
    uint32_t       hash_val;
    ht_bucket_t    *ht_bucket;
    void           *key;

    HAL_ASSERT_RETURN(((entry != NULL) && (ht_ctxt != NULL)),
                      HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN(entry != ht_ctxt->entry, HAL_RET_INVALID_ARG);

    key = get_key_func_(entry);
    HAL_ASSERT_RETURN(key != NULL, HAL_RET_ERR);
    hash_val = hash_func_(key, num_buckets_);
    HAL_ASSERT_RETURN((hash_val < num_buckets_), HAL_RET_ERR);

    HAL_ATOMIC_INC_UINT32(&this->num_removals_, 1);
    ht_bucket = &this->ht_buckets_[hash_val];
    if (thread_safe_) {
        HAL_ASSERT_RETURN((HAL_SPINLOCK_LOCK(&ht_bucket->slock_) == 0),
                          HAL_RET_ERR);
    }

    if (ht_bucket->ht_ctxt == ht_ctxt) {
        ht_bucket->ht_ctxt = ht_ctxt->next;
        ht_bucket->ht_ctxt->prev = NULL;
    } else if (ht_ctxt->next == NULL) {
        ht_ctxt->prev->next = NULL;
    } else {
        ht_ctxt->prev->next = ht_ctxt->next;
        ht_ctxt->next->prev = ht_ctxt->prev;
    }
    ht_ctxt->reset();
    ht_bucket->num_entries--;
    if (thread_safe_) {
        HAL_ASSERT_RETURN((HAL_SPINLOCK_UNLOCK(&ht_bucket->slock_) == 0),
                          HAL_RET_ERR);
    }
    HAL_ATOMIC_DEC_UINT32(&num_entries_, 1);
    return HAL_RET_OK;
}

}    // namespace utils
}    // namespace hal
