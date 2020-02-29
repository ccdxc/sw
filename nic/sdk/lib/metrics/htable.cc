// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

// this file contains shared memory based hash table implementation

#include <string.h>

#include "htable.hpp"
#include "shm.hpp"

namespace sdk {
namespace metrics {

// TableMgr constructor
TableMgr::TableMgr(htable_t *htable, ShmPtr shm_ptr) {
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

// insertHashEntry inserts an entry into hash table
error TableMgr::insertHashEntry(const char *key, int16_t keylen, ht_entry_t *entry) {
    // compute hash on the key
    uint32_t hash = fnv_hash(key, keylen);
    // calculate the hash bucket index
    int32_t idx = (int32_t)(hash % ht_->num_buckets);

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

    return error::OK();
}

// Create creates an entry in the hash table for a key
// its caller's responsibility to fill the value
void * TableMgr::Create(const char *key, int16_t keylen, int16_t val_len) {
    // basic validation
    if ((key == NULL) || (keylen <= 0) || (keylen > MAX_KEY_LEN)) {
        //LogError("Invalid key {p} or keylen {}", key, keylen);
        return NULL;
    }

    // check if the key already exists
    // FIXME: this is little inefficient(i.e, lookup twice)
    void *val = this->Find(key, keylen);
    if (val != NULL) {
        this->Release(val); // so that we dont double ref count it
        return val;
    }

    // create hash entry
    ht_entry_t *entry = this->createHashEntry(key, keylen, val_len);
    if (entry == NULL) {
        //LogError("Error allocating memory");
        return NULL;
    }

    //LogDebug("Creating key {} in table {}, offset 0x{}", key, ht_->tbl_name, OFFSET_FROM_PTR(shm_ptr_->GetBase(), entry));

    // insert the hash entry into table
    error err = this->insertHashEntry(key, keylen, entry);
    assert(err.IsOK());

    assert(((int64_t)VAL_PTR_FROM_HASH_ENTRY(entry) & 0x07) == 0);
    return (void *)VAL_PTR_FROM_HASH_ENTRY(entry);
}

// CreateDpstats creates hash entry pointing to datapath stats in PAL memory
char * TableMgr::CreateDpstats(const char *key, int16_t keylen, uint64_t pal_addr,
                               int16_t val_len) {
    // basic validation
    if ((key == NULL) || (keylen <= 0) || (keylen > MAX_KEY_LEN)) {
        //LogError("Invalid key {p} or keylen {}", key, keylen);
        return NULL;
    }

    // check if the key already exists
    // FIXME: this is little inefficient(i.e, lookup twice)
    void *val = this->Find(key, keylen);
    if (val != NULL) {
        this->Release(val); // so that we dont double ref count it
        return (char *)val;
    }

    // create hash entry
    ht_entry_t *entry = this->createHashEntry(key, keylen, sizeof(pal_addr));
    if (entry == NULL) {
        //LogError("Error allocating space in shared memory");
        return NULL;
    }

    // set ht_entry params
    entry->flags |= HT_ENTRY_FLAG_DPSTATS;
    entry->val_len = val_len; // write PAL stats len here

    //LogDebug("Creating key {} in table {}, offset 0x{}", key, ht_->tbl_name, OFFSET_FROM_PTR(shm_ptr_->GetBase(), entry));

    // insert the hash entry into table
    error err = this->insertHashEntry(key, keylen, entry);
    assert(err.IsOK());

    // copy the pal address into value part
    assert(((int64_t)VAL_PTR_FROM_HASH_ENTRY(entry) & 0x07) == 0);
    assert((pal_addr & 0x07) == 0);
    uint64_t *val_ptr = (uint64_t *)VAL_PTR_FROM_HASH_ENTRY(entry);
    *val_ptr = pal_addr;

   return (char *)VAL_PTR_FROM_HASH_ENTRY(entry);
}

// Publish atomically publishes an entry into hash table
error TableMgr::Publish(const char *key, int16_t keylen, const char *val, int16_t val_len) {
    error err;

    // basic validation
    if ((key == NULL) || (keylen <= 0) || (keylen > MAX_KEY_LEN)) {
        //LogError("Invalid key {p} or keylen {}", key, keylen);
        return NULL;
    }


    // create hash entry
    ht_entry_t *entry = this->createHashEntry(key, keylen, val_len);
    if (entry == NULL) {
        //LogError("Error allocating memory");
        return error::New("Error allocating memory");
    }

    assert(((int64_t)VAL_PTR_FROM_HASH_ENTRY(entry) & 0x07) == 0);
    // copy value
    memcpy(VAL_PTR_FROM_HASH_ENTRY(entry), val, val_len);

    //LogDebug("Publishing key {} in table {}, offset 0x{}", key, ht_->tbl_name, OFFSET_FROM_PTR(shm_ptr_->GetBase(), entry));

    // if old entry exists, atomically swap to new entry
    ht_entry_t *old_entry = this->findEntry(key, keylen);
    if (old_entry != NULL) {
        auto old_val_ptr = VAL_PTR_FROM_HASH_ENTRY(old_entry);
        Release(old_val_ptr);

        // atomically swap old entry with new
        return swapHashEntry(entry, key, keylen);
    }

    // insert the hash entry into table
    err = this->insertHashEntry(key, keylen, entry);
    assert(err.IsOK());


    return err;
}

// createHashEntry allocates memory for hash entry and initializes it
ht_entry_t *TableMgr::createHashEntry(const char *key, int16_t keylen, int16_t val_len) {
    // alloc memory for hash entry
    int mem_size = sizeof(ht_entry_t) + ROUND_KEYLAN(keylen) +
                    sizeof(ht_entry_trailer_t) + val_len;
    void *ptr = shm_ptr_->Alloc(mem_size);
    if (ptr == NULL) {
        //LogError("Error allocating memory");
        return NULL;
    }

    // initialize the hash entry
    ht_entry_t *entry = (ht_entry_t *)ptr;
    entry->next_entry = 0;
    entry->flags   = 0;
    entry->key_len = keylen;
    entry->val_len = val_len;
    // copy the key at the end of the hash entry
    memcpy(((int8_t *)ptr + sizeof(ht_entry_t)), key, keylen);

    // initialize the trailer
    ht_entry_trailer_t *trailer = TRAILER_FROM_HASH_ENTRY(entry);
    trailer->refcnt = 1;
    trailer->ht_entry = OFFSET_FROM_PTR(shm_ptr_->GetBase(), entry);
    assert(((int64_t)VAL_PTR_FROM_HASH_ENTRY(entry) & 0x07) == 0);
    memset(VAL_PTR_FROM_HASH_ENTRY(entry), 0, val_len);
    return entry;
}

// Find finds an entry in the hash table by its key
void * TableMgr::Find(const char *key, int16_t keylen) {
    // basic validation
    if ((key == NULL) || (keylen <= 0) || (keylen > MAX_KEY_LEN)) {
        //LogError("Invalid key {p} or keylen {}", key, keylen);
        return NULL;
    }

    // find the entry
    ht_entry_t *entry = this->findEntry(key, keylen);
    if (entry != NULL) {
        assert(((int64_t)VAL_PTR_FROM_HASH_ENTRY(entry) & 0x07) == 0);
        return (void *)VAL_PTR_FROM_HASH_ENTRY(entry);
    }

    return NULL;
}

// FindDpstats returns PAL address for Dpstats
// This is meant for stats stored in PAL memory by datapath
uint64_t TableMgr::FindDpstats(const char *key, int16_t keylen) {
    // basic validation
    if ((key == NULL) || (keylen <= 0) || (keylen > MAX_KEY_LEN)) {
        //LogError("Invalid key {p} or keylen {}", key, keylen);
        return 0;
    }

    // find the entry
    ht_entry_t *entry = this->findEntry(key, keylen);
    if ((entry != NULL) && (entry->flags & HT_ENTRY_FLAG_DPSTATS)) {
        assert(((int64_t)VAL_PTR_FROM_HASH_ENTRY(entry) & 0x07) == 0);
        uint64_t *vptr = (uint64_t *)VAL_PTR_FROM_HASH_ENTRY(entry);
        assert((*vptr & 0x07) == 0);
        return *vptr;
    }

    // this is error case, returning 0 means we didnt find the entry
    return 0;
}

ht_entry_t * TableMgr::findEntry(const char *key, int16_t keylen) {
    ht_entry_t *entry = NULL;

    // compute hash on the key
    uint32_t hash = fnv_hash(key, keylen);
    // calculate the hash bucket index
    int32_t idx = (int32_t) (hash % ht_->num_buckets);

    // check if this is single level or multi level table
    if (!HTABLE_IS_TWO_LEVEL(ht_)) {
        ht_bucket_t *bkt = &ht_->buckets[idx];
        spin_lock(&bkt->rw_lock);
        if (bkt->ht_entry != 0) {
            entry =  findMatchingEntry(bkt->ht_entry, key, keylen);
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
            entry =  findMatchingEntry(bkt->ht_entry, key, keylen);
        }
        spin_unlock(&bkt->rw_lock);
    }

    return entry;
}

// findMatchingEntry traverses linked list of hash entries and finds a match
ht_entry_t * TableMgr::findMatchingEntry(int32_t offset, const char *key, int16_t keylen) {
    // traverse the linked list till we find a match
    while (offset != 0) {
        ht_entry_t *entry = (ht_entry_t *)PTR_FROM_OFFSET(shm_ptr_->GetBase(), offset);
        int8_t *hkey = (int8_t *)entry + sizeof(ht_entry_t);

        // see if the key matches
        if ((entry->key_len == keylen) && (!memcmp(hkey, key, keylen))) {
            // increment ref count
            ht_entry_trailer_t *trailer = TRAILER_FROM_HASH_ENTRY(entry);
            atomic_increment(&trailer->refcnt);

            // we found the match, return the value
            return entry;
        }

        offset = entry->next_entry;
    }

    return NULL;
}

// swapHashEntry atomically swaps an entry in hash table with new one
error TableMgr::swapHashEntry(ht_entry_t *entry, const char *key, int16_t keylen) {
    // compute hash on the key
    uint32_t hash = fnv_hash(key, keylen);
    // calculate the hash bucket index
    int32_t idx = (int32_t)(hash % ht_->num_buckets);

    // check if this is single level or multi level table
    if (!HTABLE_IS_TWO_LEVEL(ht_)) {
        ht_bucket_t *bkt = &ht_->buckets[idx];
        spin_lock(&bkt->rw_lock);
        if (bkt->ht_entry != 0) {
            int32_t *offset = &bkt->ht_entry;
            while (*offset != 0) {
                ht_entry_t *curr_entry = (ht_entry_t *)PTR_FROM_OFFSET(shm_ptr_->GetBase(), *offset);
                int8_t *hkey = (int8_t *)curr_entry + sizeof(ht_entry_t);
                if ((curr_entry->key_len == keylen) && (!memcmp(hkey, key, keylen))) {
                    // swap the entry in linked list
                    entry->next_entry = curr_entry->next_entry;
                    *offset = OFFSET_FROM_PTR(shm_ptr_->GetBase(), entry);
                    curr_entry->next_entry = 0;

                    // decrement ref count and free memory
                    ht_entry_trailer_t *trailer = TRAILER_FROM_HASH_ENTRY(curr_entry);
                    int refcnt = atomic_decrement(&trailer->refcnt);
                    if (refcnt <= 1) {
                        // free the memory
                        shm_ptr_->Free(curr_entry);
                    }
                    break;
                }

                offset = &curr_entry->next_entry;
            }
        } else {
            error err = atomicInsert(bkt, entry);
            assert(err.IsOK());
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
            int32_t *offset = &bkt->ht_entry;
            while (*offset != 0) {
                ht_entry_t *curr_entry = (ht_entry_t *)PTR_FROM_OFFSET(shm_ptr_->GetBase(), *offset);
                int8_t *hkey = (int8_t *)curr_entry + sizeof(ht_entry_t);
                if ((curr_entry->key_len == keylen) && (!memcmp(hkey, key, keylen))) {
                    // swap the entry in linked list
                    entry->next_entry = curr_entry->next_entry;
                    *offset = OFFSET_FROM_PTR(shm_ptr_->GetBase(), entry);
                    curr_entry->next_entry = 0;

                    // decrement ref count and free memory
                    ht_entry_trailer_t *trailer = TRAILER_FROM_HASH_ENTRY(curr_entry);
                    int refcnt = atomic_decrement(&trailer->refcnt);
                    if (refcnt <= 1) {
                        // free the memory
                        shm_ptr_->Free(curr_entry);
                    }
                    break;
                }

                offset = &curr_entry->next_entry;
            }
        } else {
            error err = atomicInsert(bkt, entry);
            assert(err.IsOK());
        }
        spin_unlock(&bkt->rw_lock);
    }

    return error::OK();
}

// Acquire aincrements the refcount
error TableMgr::Acquire(void *val_ptr) {
    assert(((int64_t)val_ptr & 0x07) == 0);
    ht_entry_trailer_t *trailer = TRAILER_FROM_VAL_PTR(val_ptr);
    atomic_increment(&trailer->refcnt);

    return error::OK();
}

// Release releases a hash entry, memory is freed when all users release a hash entry
error TableMgr::Release(void *val_ptr) {
    assert(((int64_t)val_ptr & 0x07) == 0);
    ht_entry_trailer_t *trailer = TRAILER_FROM_VAL_PTR(val_ptr);
    int refcnt = atomic_decrement(&trailer->refcnt);
    if (refcnt <= 1) {
        ht_entry_t *entry = (ht_entry_t *)PTR_FROM_OFFSET(shm_ptr_->GetBase(), trailer->ht_entry);
        // free the memory
        return shm_ptr_->Free(entry);
    }

    return error::OK();
}

// RefCount returns the refcount of a hash entry
// Note: to be used for testing pueposes only
int32_t TableMgr::RefCount(void *val_ptr) {
    assert(((int64_t)val_ptr & 0x07) == 0);
    ht_entry_trailer_t *trailer = TRAILER_FROM_VAL_PTR(val_ptr);
    return trailer->refcnt;
}

// Delete deletes an entry by key
error TableMgr::Delete(const char *key, int16_t keylen) {
    error err;

    // basic validation
    if ((key == NULL) || (keylen <= 0) || (keylen > MAX_KEY_LEN)) {
        //LogError("Invalid key {p} or keylen {}", key, keylen);
        return NULL;
    }

    //LogDebug("Deleting key {} in table {}", key, ht_->tbl_name);

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
        if ((entry->key_len == keylen) && (!memcmp(hkey, key, keylen))) {
            // remove from the linked list
            *offset = entry->next_entry;
            entry->next_entry = 0;
            ht_->num_entries--;

            // decrement ref count
            ht_entry_trailer_t *trailer = TRAILER_FROM_HASH_ENTRY(entry);
            int refcnt = atomic_decrement(&trailer->refcnt);
            if (refcnt <= 1) {
                // free the memory
                return shm_ptr_->Free(entry);
            }

            return error::OK();
        }

        offset = &entry->next_entry;
    }

    return error::New("Key not found");
}

// getNextEntry returns the next valid entry from a hash index
ht_entry_t * TableMgr::getNextEntry(int idx) {
    if (!HTABLE_IS_TWO_LEVEL(ht_)) {
        for (; idx < ht_->num_buckets; idx++) {
            ht_bucket_t *bkt = &ht_->buckets[idx];
            spin_lock(&bkt->rw_lock);
            if (bkt->ht_entry != 0) {
                ht_entry_t *entry = (ht_entry_t *)PTR_FROM_OFFSET(shm_ptr_->GetBase(), bkt->ht_entry);
                // increment ref count
                ht_entry_trailer_t *trailer = TRAILER_FROM_HASH_ENTRY(entry);
                atomic_increment(&trailer->refcnt);
                spin_unlock(&bkt->rw_lock);
                return entry;
            }
            spin_unlock(&bkt->rw_lock);
        }
    } else {
        for (; idx < ht_->num_buckets; idx++) {
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
                ht_entry_t *entry = (ht_entry_t *)PTR_FROM_OFFSET(shm_ptr_->GetBase(), bkt->ht_entry);
                // increment ref count
                ht_entry_trailer_t *trailer = TRAILER_FROM_HASH_ENTRY(entry);
                atomic_increment(&trailer->refcnt);
                spin_unlock(&bkt->rw_lock);
                return entry;
            }
            spin_unlock(&bkt->rw_lock);
        }
    }

    return NULL;
}

ht_entry_t * TableMgr::GetNext(ht_entry_t *entry) {
    // if table is empty, nothing to return
    if (this->ht_->num_entries == 0) {
        return NULL;
    }

    // if previous was null, return the first entry in table
    if (entry == NULL) {
        return getNextEntry(0);
    }

    // compute hash on the key
    uint32_t hash = fnv_hash(KEY_PTR_FROM_HASH_ENTRY(entry), entry->key_len);
    // calculate the hash bucket index
    int32_t idx = (int32_t)(hash % ht_->num_buckets);

    ht_bucket_t *bkt = &ht_->buckets[idx];
    spin_lock(&bkt->rw_lock);

    // if the hash entry has a next element, return it
    if (entry->next_entry != 0) {
        auto next_entry = (ht_entry_t *)PTR_FROM_OFFSET(shm_ptr_->GetBase(), entry->next_entry);

        // increment ref count for next entry
        ht_entry_trailer_t *trailer = TRAILER_FROM_HASH_ENTRY(next_entry);
        atomic_increment(&trailer->refcnt);

        // decrement ref count for old entry
        Release(VAL_PTR_FROM_HASH_ENTRY(entry));
        spin_unlock(&bkt->rw_lock);

        return next_entry;
    }

    // decrement ref count
    Release(VAL_PTR_FROM_HASH_ENTRY(entry));
    spin_unlock(&bkt->rw_lock);

    // find the next entry in hash table
    return getNextEntry(++idx);
}

// Iterator returns an iterator for the table
TableIterator TableMgr::Iterator() {
     return TableIterator(this);
}

// TableIterator constructor
TableIterator::TableIterator(TableMgr *tbl) {
    tbl_ = tbl;
    entry_ = tbl->GetNext(NULL);
}

// Next gets next entry in the table
void TableIterator::Next() {
    entry_ = tbl_->GetNext(entry_);
}

// DumpEntry dumps information about a hash entry
void TableMgr::DumpEntry(const char *key, int16_t keylen) {
    ht_entry_t *entry = this->findEntry(key, keylen);
    if (entry != NULL) {
        uint8_t *valptr = (uint8_t *)VAL_PTR_FROM_HASH_ENTRY(entry);
        uint8_t *keyptr = (uint8_t *)entry + sizeof(ht_entry_t);
        ht_entry_trailer_t *trailer = TRAILER_FROM_HASH_ENTRY(entry);
        printf("Keylen: %d, Vallen: %d, refcnt: %d\n", entry->key_len, entry->val_len, trailer->refcnt);
        printf("key: ");
        for (int i = 0; i < entry->key_len; i++) {
            printf("%02x ", keyptr[i]);
        }
        if (entry->flags & HT_ENTRY_FLAG_DPSTATS) {
            printf("\nPal addr: 0x%lx\n", *(uint64_t *)valptr);
        } else {
            printf("\nValue: ");
            for (int i = 0; i < entry->val_len; i++) {
                printf("%02x ", valptr[i]);
            }
            printf("\n");
        }
        Release(valptr);
    } else {
        printf("hash entry not found\n");
    }
}

// DumpTable prints the contents of the table
void TableMgr::DumpTable() {
    ht_entry_t *entry = NULL;
    printf("Table: %s, buckets: %d, flags: %x, tablets: %d, entries: %d, used buckets: %d\n",
        ht_->tbl_name, ht_->num_buckets, ht_->ht_flags, ht_->num_tablets,
        ht_->num_entries, ht_->num_used_buckets);

    // walk all entries and dump it
    while ((entry = GetNext(entry)) != NULL) {
        uint32_t hash = fnv_hash(KEY_PTR_FROM_HASH_ENTRY(entry), entry->key_len);
        int32_t idx = (int32_t)(hash % ht_->num_buckets);
        uint8_t *keyptr = (uint8_t *)entry + sizeof(ht_entry_t);
        uint8_t *valptr = (uint8_t *)VAL_PTR_FROM_HASH_ENTRY(entry);
        ht_entry_trailer_t *trailer = TRAILER_FROM_HASH_ENTRY(entry);
        printf("  Idx: %d, Keylen: %d, Vallen: %d, refcnt: %d\n", idx, entry->key_len, entry->val_len, trailer->refcnt);
        printf("    key: ");
        for (int i = 0; i < entry->key_len; i++) {
            printf("%02x ", keyptr[i]);
        }
        printf("\n");
        if (entry->flags & HT_ENTRY_FLAG_DPSTATS) {
            printf("    Pal addr: 0x%lx\n", *(uint64_t *)valptr);
        } else {
            printf("    Value: ");
            for (int i = 0; i < entry->val_len; i++) {
                printf("%02x ", valptr[i]);
            }
            printf("\n");
        }
    }
}

} // namespace metrics
} // namespace sdk
