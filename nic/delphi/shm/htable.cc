// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

// this file contains shared memory based hash table implementation

#include "htable.hpp"
#include "shm.hpp"

namespace delphi {
namespace shm {

// TableMgr constructor
TableMgr::TableMgr(htable_t *htable, DelphiShmPtr shm_ptr) {
    ht_      = htable;
    shm_ptr_ = shm_ptr;
}

// atomicInsert inserts into hash bucket using CAS operation.
// it keeps retrying till it suceeds
error TableMgr::atomicInsert(ht_bucket_t *bkt, ht_entry_t *entry) {
    bool done = false;
    int32_t offset = OFFSET_FROM_PTR(shm_ptr_->GetBase(), entry);

    // keep retrying insert
    while (!done) {
        entry->next_entry = bkt->ht_entry;
        done = atomic_compare_and_swap(&bkt->ht_entry, bkt->ht_entry, offset);
    }

    return error::OK();
}

// Create creates an entry in the hash table for a key
// its caller's responsibility to fill the value
void * TableMgr::Create(const char *key, int16_t keylen, int16_t val_len) {
    // compute hash on the key
    uint32_t hash = fnv_hash(key, keylen);
    // calculate the hash bucket index
    int32_t idx = (int32_t)(hash % ht_->num_buckets);

    // check if the key already exists
    // FIXME: this is little inefficient(i.e, lookup twice)
    void *val = Find(key, keylen);
    if (val != NULL) {
        return val;
    }

    // create hash entry
    ht_entry_t *entry = createHashEntry(key, keylen, val_len);
    if (entry == NULL) {
        LogError("Error allocating memory");
        return NULL;
    }

    LogDebug("Creating key {} in table {} at idx {}, offset 0x{}", key, ht_->tbl_name, idx, OFFSET_FROM_PTR(shm_ptr_->GetBase(), entry));

    // check if this is single level or multi level table
    if (!HTABLE_IS_TWO_LEVEL(ht_)) {
        ht_bucket_t *bkt = &ht_->buckets[idx];
        spin_lock(&bkt->rw_lock);

        // some book keeping
        if (bkt->ht_entry == 0) {
            // FIXME: does this need to be atomic
            ht_->num_used_buckets++;
        }

        // insert the entry into embedded hash buckets
        error err = atomicInsert(bkt, entry);
        assert(err.IsOK());
        spin_unlock(&bkt->rw_lock);
    } else {
        // calculate tablet index and bucket index
        int32_t tablet_idx = idx / ht_->num_buckets_per_tablet;
        int32_t bkt_idx = idx % ht_->num_buckets_per_tablet;

        // get a pointer to the tablet
        assert(tablet_idx < ht_->num_tablets);
        assert(ht_->buckets[tablet_idx].ht_entry != 0);
        auto tptr = PTR_FROM_OFFSET(shm_ptr_->GetBase(), ht_->buckets[tablet_idx].ht_entry);
        ht_tablet_t *tablet = (ht_tablet_t *)tptr;

        ht_bucket_t *bkt = &tablet->buckets[bkt_idx];
        spin_lock(&bkt->rw_lock);

        // some book keeping
        if (bkt->ht_entry == 0) {
            ht_->num_used_buckets++;
        }

        // insert the entry into the tablet
        error err = atomicInsert(bkt, entry);
        assert(err.IsOK());
        spin_unlock(&bkt->rw_lock);
    }

    ht_->num_entries++;

    return (void *)VAL_PTR_FROM_HASH_ENTRY(entry);
}

// createHashEntry allocates memory for hash entry and initializes it
ht_entry_t *TableMgr::createHashEntry(const char *key, int16_t keylen, int16_t val_len) {
    // alloc memory for hash entry
    int mem_size = sizeof(ht_entry_t) + ROUND_KEYLAN(keylen) +
                    sizeof(ht_entry_trailer_t) + val_len;
    void *ptr = shm_ptr_->Alloc(mem_size);
    if (ptr == NULL) {
        LogError("Error allocating memory");
        return NULL;
    }

    // initialize the hash entry
    ht_entry_t *entry = (ht_entry_t *)ptr;
    entry->next_entry = 0;
    entry->key_len = keylen;
    entry->val_len = val_len;
    // copy the key at the end of the hash entry
    memcpy(((int8_t *)ptr + sizeof(ht_entry_t)), key, keylen);

    // initialize the trailer
    ht_entry_trailer_t *trailer = TRAILER_FROM_HASH_ENTRY(entry);
    trailer->refcnt = 1;
    trailer->ht_entry = OFFSET_FROM_PTR(shm_ptr_->GetBase(), entry);

    return entry;
}

// Find finds an entry in the hash table by its key
void * TableMgr::Find(const char *key, int16_t keylen) {
    void *valptr = NULL;
    // compute hash on the key
    uint32_t hash = fnv_hash(key, keylen);
    // calculate the hash bucket index
    int32_t idx = (int32_t) (hash % ht_->num_buckets);

    // check if this is single level or multi level table
    if (!HTABLE_IS_TWO_LEVEL(ht_)) {
        ht_bucket_t *bkt = &ht_->buckets[idx];
        spin_lock(&bkt->rw_lock);
        if (bkt->ht_entry != 0) {
            valptr =  findMatchingEntry(bkt->ht_entry, key, keylen);
        }
        spin_unlock(&bkt->rw_lock);
    } else {
        // calculate tablet index and bucket index
        int32_t tablet_idx = idx / ht_->num_buckets_per_tablet;
        int32_t bkt_idx = idx % ht_->num_buckets_per_tablet;

        // get a pointer to the tablet
        assert(tablet_idx < ht_->num_tablets);
        assert(ht_->buckets[tablet_idx].ht_entry != 0);
        auto tptr = PTR_FROM_OFFSET(shm_ptr_->GetBase(), ht_->buckets[tablet_idx].ht_entry);
        ht_tablet_t *tablet = (ht_tablet_t *)tptr;

        ht_bucket_t *bkt = &tablet->buckets[bkt_idx];
        spin_lock(&bkt->rw_lock);

        if (bkt->ht_entry != 0) {
            valptr =  findMatchingEntry(bkt->ht_entry, key, keylen);
        }
        spin_unlock(&bkt->rw_lock);
    }

    return valptr;
}

// findMatchingEntry traverses linked list of hash entries and finds a match
void * TableMgr::findMatchingEntry(int32_t offset, const char *key, int16_t keylen) {
    // traverse the linked list till we find a match
    while (offset != 0) {
        ht_entry_t *entry = (ht_entry_t *)PTR_FROM_OFFSET(shm_ptr_->GetBase(), offset);
        int8_t *hkey = (int8_t *)entry + sizeof(ht_entry_t);

        // see if the key matches
        if (!memcmp(hkey, key, keylen)) {
            // increment ref count
            ht_entry_trailer_t *trailer = TRAILER_FROM_HASH_ENTRY(entry);
            atomic_increment(&trailer->refcnt);

            // we found the match, return the value
            return (void *)VAL_PTR_FROM_HASH_ENTRY(entry);
        }

        offset = entry->next_entry;
    }

    return NULL;
}

// Release releases a hash entry, memory is freed when all users release a hash entry
error TableMgr::Release(void *val_ptr) {
    ht_entry_trailer_t *trailer = TRAILER_FROM_VAL_PTR(val_ptr);
    atomic_decrement(&trailer->refcnt);
    if (trailer->refcnt <= 0) {
        ht_entry_t *entry = (ht_entry_t *)PTR_FROM_OFFSET(shm_ptr_->GetBase(), trailer->ht_entry);
        // free the memory
        return shm_ptr_->Free(entry);
    }

    return error::OK();
}

// RefCount returns the refcount of a hash entry
// Note: to be used for testing pueposes only
int32_t TableMgr::RefCount(void *val_ptr) {
    ht_entry_trailer_t *trailer = TRAILER_FROM_VAL_PTR(val_ptr);
    return trailer->refcnt;
}

// Delete deletes an entry by key
error TableMgr::Delete(const char *key, int16_t keylen) {
    error err;

    // compute hash on the key
    uint32_t hash = fnv_hash(key, keylen);
    // calculate the hash bucket index
    int32_t idx = (int32_t)(hash % ht_->num_buckets);

    // check if this is single level or multi level table
    if (!HTABLE_IS_TWO_LEVEL(ht_)) {
        ht_bucket_t *bkt = &ht_->buckets[idx];
        spin_lock(&bkt->rw_lock);
        if (bkt->ht_entry != 0) {
            // delete the entry
            err =  deleteMatchingEntry(&ht_->buckets[idx].ht_entry, key, keylen);

            // if hash bucket became empty, decrement counter
            if (bkt->ht_entry == 0) {
                ht_->num_used_buckets--;
            }
        }
        spin_unlock(&bkt->rw_lock);
    } else {
        // calculate tablet index and bucket index
        int32_t tablet_idx = idx / ht_->num_buckets_per_tablet;
        int32_t bkt_idx = idx % ht_->num_buckets_per_tablet;

        // get a pointer to the tablet
        assert(tablet_idx < ht_->num_tablets);
        assert(ht_->buckets[tablet_idx].ht_entry != 0);
        auto tptr = PTR_FROM_OFFSET(shm_ptr_->GetBase(), ht_->buckets[tablet_idx].ht_entry);
        ht_tablet_t *tablet = (ht_tablet_t *)tptr;

        ht_bucket_t *bkt = &tablet->buckets[bkt_idx];
        spin_lock(&bkt->rw_lock);
        if (bkt->ht_entry != 0) {
            // remove the entry
            err = deleteMatchingEntry(&bkt->ht_entry, key, keylen);

            // if hash bucket became empty, decrement counter
            if (bkt->ht_entry == 0) {
                ht_->num_used_buckets--;
            }
        }
        spin_unlock(&bkt->rw_lock);
    }

    return err;
}

// deleteMatchingEntry traverses the linked list and deletes the matching entry
error TableMgr::deleteMatchingEntry(int32_t *offset, const char *key, int16_t keylen) {
    // traverse the linked list till we find a match
    while (*offset != 0) {
        ht_entry_t *entry = (ht_entry_t *)PTR_FROM_OFFSET(shm_ptr_->GetBase(), *offset);
        int8_t *hkey = (int8_t *)entry + sizeof(ht_entry_t);
        if (!memcmp(hkey, key, keylen)) {
            // remove from the linked list
            *offset = entry->next_entry;

            ht_->num_entries--;

            // decrement ref count
            ht_entry_trailer_t *trailer = TRAILER_FROM_HASH_ENTRY(entry);
            atomic_decrement(&trailer->refcnt);
            if (trailer->refcnt <= 0) {
                // free the memory
                return shm_ptr_->Free(entry);
            }

            return error::OK();
        }

        offset = &entry->next_entry;
    }

    return error::New("Key not found");
}

} // namespace shm
} // namespace delphi
