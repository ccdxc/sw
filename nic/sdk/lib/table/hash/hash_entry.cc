//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <cstring>
#include "hash_entry.hpp"
#include "sdk/mem.hpp"

namespace sdk {
namespace table {

//---------------------------------------------------------------------------
// returns key for hash table entry
//---------------------------------------------------------------------------
void *
hash_entry_get_key_func (void *entry)
{
    hash_entry_t *hash_entry = (hash_entry_t *)entry;

    return (void *)&hash_entry->index;
}

//---------------------------------------------------------------------------
// compute hash 
//---------------------------------------------------------------------------
uint32_t 
hash_entry_compute_hash_func(void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(uint32_t)) % ht_size;
}

//---------------------------------------------------------------------------
// compare keys
//---------------------------------------------------------------------------
bool 
hash_entry_compare_key_func (void *key1, void *key2)
{
    if (*(uint32_t *)key1 == *(uint32_t *)key2) {
        return true;
    }
    return false;
}

hash_entry_t *
hash_entry_create(void *key, uint32_t key_len,
                  void *data, uint32_t data_len, uint32_t index)
{
    hash_entry_t *te  = NULL;

    te = (hash_entry_t *)SDK_CALLOC(SDK_MEM_ALLOC_ID_HASH_ENTRY, 
                                    sizeof(hash_entry_t));
    if (!te) {
        return NULL;
    }

    te->key_len  = key_len;
    te->data_len = data_len;
    te->index    = index;

    te->key      = SDK_MALLOC(SDK_MEM_ALLOC_ID_HASH_ENTRY_KEY, key_len);
    te->data     = SDK_MALLOC(SDK_MEM_ALLOC_ID_HASH_ENTRY_DATA, data_len);

    memcpy(te->key, key, key_len);
    memcpy(te->data, data, data_len);

    return te;
}

void
hash_entry_delete(hash_entry_t *te)
{
    SDK_FREE(SDK_MEM_ALLOC_ID_HASH_ENTRY_KEY, te->key);
    SDK_FREE(SDK_MEM_ALLOC_ID_HASH_ENTRY_DATA, te->data);
    SDK_FREE(SDK_MEM_ALLOC_ID_HASH_ENTRY, te);
}

// ---------------------------------------------------------------------------
// updates data
// ---------------------------------------------------------------------------
void
hash_entry_update_data(hash_entry_t *he, void *data)
{
    memcpy(he->data, data, he->data_len);
}

#if 0
//---------------------------------------------------------------------------
// factory method to instantiate the class
//---------------------------------------------------------------------------
hash_entry *
hash_entry::factory(void *key, uint32_t key_len, void *data,
                   uint32_t data_len, uint32_t index)
{
    void        *mem = NULL;
    hash_entry   *he = NULL;

    mem = SDK_CALLOC(SDK_MEM_ALLOC_ID_HASH_ENTRY, sizeof(hash_entry));
    if (!mem) {
        return NULL;
    }

    he = new (mem) hash_entry(key, key_len, data, data_len, index);
    return he;
}

//---------------------------------------------------------------------------
// method to free & delete the object
//---------------------------------------------------------------------------
void
hash_entry::destroy(hash_entry *he)
{
    if (he) {
        he->~hash_entry();
        SDK_FREE(SDK_MEM_ALLOC_ID_HASH_ENTRY, he);
    }
}


// ---------------------------------------------------------------------------
// constructor - hash_entry
// ---------------------------------------------------------------------------
hash_entry::hash_entry(void *key, uint32_t key_len, 
                     void *data, uint32_t data_len, uint32_t index)
{
    key_len_    = key_len;
    data_len_   = data_len;
    index_      = index;

    key_ = SDK_MALLOC(SDK_MEM_ALLOC_ID_HASH_ENTRY_KEY, key_len);
    data_ = SDK_MALLOC(SDK_MEM_ALLOC_ID_HASH_ENTRY_DATA, data_len);

    std::memcpy(key_, key, key_len);
    std::memcpy(data_, data, data_len);
}

// ---------------------------------------------------------------------------
// destructor - hash_entry
// ---------------------------------------------------------------------------
hash_entry::~hash_entry()
{
    SDK_FREE(SDK_MEM_ALLOC_ID_HASH_ENTRY_KEY, key_);
    SDK_FREE(SDK_MEM_ALLOC_ID_HASH_ENTRY_DATA, data_);
}

// ---------------------------------------------------------------------------
// updates data
// ---------------------------------------------------------------------------
void
hash_entry::update_data(void *data)
{
    std::memcpy(data_, data, data_len_);
}
#endif

}   // namespace table
}   // namespace sdk

