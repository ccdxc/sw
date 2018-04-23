// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_SHM_HTABLE_H_
#define _DELPHI_SHM_HTABLE_H_

#include "nic/delphi/utils/utils.hpp"
#include "shm_types.hpp"

namespace delphi {
namespace shm {

// ROUND_KEYLAN rounds key length to next 64bit boundary
#define ROUND_KEYLAN(len) ((len + 7) & 0xFFF8)

// MAX_SEQ_BUCKETS max number of buckets in a contiguous table
#define MAX_SEQ_BUCKETS (2500)

// TABLE_NAME_LEN is max length of table name (including '\0')
#define TABLE_NAME_LEN 128

// ht_entry_t represents an entry in the hash table
// if a hash bucket has multiple entries, they'll be linked using a linked list
//
// each hash entry looks like:
//
//   +--------------+
//   |  ht_entry_t  |
//   +--------------+
//   | key (rounded |
//   | to 64bit)    |
//   +--------------+
//   |    Value     |
//   +--------------+
//
typedef struct ht_entry_ {
    int32_t    next_entry; // next entry in the hash bucket
    int16_t    key_len;    // length of the key
    int16_t    val_len;    // length of the value
} PACKED ht_entry_t;

// ht_entry_trailer_t is used for getting hash entry from value pointer
typedef struct ht_entry_trailer_ {
    int32_t    refcnt;       // ref count of the hash entry
    int32_t    ht_entry;     // offset back to ht entry
} PACKED ht_entry_trailer_t;

// VAL_PTR_FROM_HASH_ENTRY returns a pointer to value part of hash entry
#define VAL_PTR_FROM_HASH_ENTRY(entry) ((int8_t *)entry + sizeof(ht_entry_t) + \
                                        ROUND_KEYLAN(entry->key_len) + sizeof(ht_entry_trailer_t))

// TRAILER_FROM_HASH_ENTRY returns pointer to trailer from hash entry
#define TRAILER_FROM_HASH_ENTRY(entry) ((ht_entry_trailer_t *)((int8_t *)entry + \
                                        sizeof(ht_entry_t) + ROUND_KEYLAN(entry->key_len)))

// get trailer and hash entry from value pointer
#define TRAILER_FROM_VAL_PTR(ptr) (ht_entry_trailer_t *)((int8_t *)ptr - sizeof(ht_entry_trailer_t))
#define HASH_ENTRY_FROM_VAL_PTR(ptr) ((ht_entry_t *)OFFSET_FROM_PTR(TRAILER_FROM_VAL_PTR(ptr)->ht_entry))

// ht_bucket_t represents a hash bucket
typedef struct ht_bucket_ {
    int32_t    rw_lock;      // lock the bucket for modifications
    int32_t    ht_entry;     // hash table entry (offset)
} PACKED ht_bucket_t;

// ht_tablet_t is one chunk of hash buckets (called second level buckets)
// when a hash table buckets dont fit in one contiguous memory, we spit the hash table
// into two or more tablets each tablet typically contains 5K buckets.
// top level hash table datastructure contains pointers to each tablet
typedef struct ht_tablet_ {
    ht_bucket_t buckets[MAX_SEQ_BUCKETS];
} ht_tablet_t;

// hash table flags
#define HTABLE_FLAG_TWO_LEVEL   0x01

#define HTABLE_IS_TWO_LEVEL(ht) (ht->ht_flags & HTABLE_FLAG_TWO_LEVEL)

// htable_t represents a hash table
//
// This is the hash table structure
//
//   +--------------+
//   |  htable_t    |
//   +--------------+
//   |  bucket 0    |
//   +--------------+
//   |  bucket 1    |
//   +--------------+
//   |    ......    |
//   +--------------+
//   |  bucket N    |
//   +--------------+
//
// a hash table can have upto 5K buckets in the first level buckets that are
// embedded into the hash table(5Kx4 = 20KB is the largest chunk size we can allocate in shared memory).
// When a hash table needs to grow bigger than 5K buckets, hash table creates
// second level buckets. In this case, first level buckets become pointers to
// second level buckets(called tablets).
//
//   +--------------+
//   |  htable_t    |
//   +--------------+                        Tablet 0
//   |  tablet 0    +--------------------->+----------+
//   +--------------+       Table 1        | Bucket 0 |
//   |  tablet 1    +-->+--------------+   +----------+
//   +--------------+   | Bucket M + 1 |   | Bucket 1 |
//   |    ......    |   +--------------+   +----------+
//   +--------------+   | Bucket M + 2 |   | .....    |
//   |  tablet N    |   +--------------+   +----------+
//   +--------------+   |   .....      |   | Bucket M |
//                      +--------------+   +----------+
//
// maximum number of buckets in tablet is also 5K. This means, maximum
// size of hash table is (5K * 5K) = 25 million hash buckets.
//
typedef struct htable_ {
    char          tbl_name[TABLE_NAME_LEN];  // table name
    int32_t       rw_lock;          // read write lock for hash table
    int32_t       num_buckets;      // number of buckets in the hash table
    int32_t       ht_flags;         // table flags
    int32_t       num_entries;      // number of valid entries
    int32_t       num_used_buckets; // used buckets (to determine collision rate)
    int32_t       num_tablets;      // number of tablets (0 if buckets are embedded below)
    int32_t       num_buckets_per_tablet;   // number of buckets per tablet
    ht_bucket_t   buckets[0];       // first level buckets
} htable_t;

// TableMgr class manages a hash table in shared memory
class TableMgr {
public:
    TableMgr(htable_t *htable, DelphiShmPtr shm_ptr);
    void *Create(const char *key, int16_t keylen, int16_t val_len);   // create an entry for the key in hash table
    void *Find(const char *key, int16_t keylen);     // finds an entry by key
    error Release(void *val_ptr);                    // release a hash entry from use
    error Delete(const char *key, int16_t keylen);   // delete an entry
    int32_t RefCount(void *val_ptr);                 // returns ref count of hash entry (for tetsing only)

private:
    htable_t      *ht_;
    DelphiShmPtr  shm_ptr_;

    // private methods
    ht_entry_t *createHashEntry(const char *key, int16_t keylen, int16_t val_len);
    void * findMatchingEntry(int32_t offset, const char *key, int16_t keylen);
    error deleteMatchingEntry(int32_t *offset, const char *key, int16_t keylen);
    error atomicInsert(ht_bucket_t *bkt, ht_entry_t *entry);

};
typedef std::unique_ptr<TableMgr> TableMgrUptr;


// FNV hash params recommended by https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
const int32_t kFnvPrime = 0x01000193; //   16777619
const int32_t kFnvSeed  = 0x811C9DC5; // 2166136261

// fnv_hash calculate FNV hash for variable sized data
inline uint32_t fnv_hash(const char *data, int16_t num_bytes)
{
    uint32_t hash = kFnvSeed;
    assert(data);
    const unsigned char* ptr = (const unsigned char*)data;
    while (num_bytes--) {
        hash = (*ptr++ ^ hash) * kFnvPrime;
    }
    return hash;
}

} // namespace shm
} // namespace delphi

#endif // _DELPHI_SHM_HTABLE_H_
