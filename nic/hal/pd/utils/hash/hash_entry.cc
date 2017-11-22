#include <cstring>

#include "nic/hal/pd/utils/hash/hash_entry.hpp"

using hal::pd::utils::HashEntry;

//---------------------------------------------------------------------------
// Factory method to instantiate the class
//---------------------------------------------------------------------------
HashEntry *
HashEntry::factory(void *key, uint32_t key_len, void *data,
                   uint32_t data_len, uint32_t index,
                   uint32_t mtrack_id)
{
    void        *mem = NULL;
    HashEntry   *he = NULL;

    mem = HAL_CALLOC(mtrack_id, sizeof(HashEntry));
    if (!mem) {
        return NULL;
    }

    he = new (mem) HashEntry(key, key_len, data, data_len, index);
    return he;
}

//---------------------------------------------------------------------------
// Method to free & delete the object
//---------------------------------------------------------------------------
void
HashEntry::destroy(HashEntry *he, uint32_t mtrack_id) 
{
    if (he) {
        he->~HashEntry();
        HAL_FREE(mtrack_id, he);
    }
}


// ---------------------------------------------------------------------------
// Constructor - HashEntry
// ---------------------------------------------------------------------------
HashEntry::HashEntry(void *key, uint32_t key_len, 
                     void *data, uint32_t data_len, uint32_t index)
{
    key_len_    = key_len;
    data_len_   = data_len;
    index_      = index;

    // key_    = ::operator new(key_len);
    // data_   = ::operator new(data_len);

    key_ = HAL_MALLOC(HAL_MEM_ALLOC_HASH_ENTRY_KEY, key_len);
    data_ = HAL_MALLOC(HAL_MEM_ALLOC_HASH_ENTRY_DATA, data_len);

    std::memcpy(key_, key, key_len);
    std::memcpy(data_, data, data_len);
}

// ---------------------------------------------------------------------------
// Destructor - HashEntry
// ---------------------------------------------------------------------------
HashEntry::~HashEntry()
{
    // ::operator delete(key_);
    // ::operator delete(data_);

    HAL_FREE(HAL_MEM_ALLOC_HASH_ENTRY_KEY, key_);
    HAL_FREE(HAL_MEM_ALLOC_HASH_ENTRY_DATA, data_);
}

// ---------------------------------------------------------------------------
// Updates Data
// ---------------------------------------------------------------------------
void
HashEntry::update_data(void *data)
{
    std::memcpy(data_, data, data_len_);
}

