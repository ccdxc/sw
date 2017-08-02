#include <cstring>

#include "hash_entry.hpp"

using hal::pd::utils::HashEntry;

// ---------------------------------------------------------------------------
// Constructor - HashEntry
// ---------------------------------------------------------------------------
HashEntry::HashEntry(void *key, uint32_t key_len, 
                     void *data, uint32_t data_len, uint32_t index)
{
    key_len_    = key_len;
    data_len_   = data_len;
    index_      = index;

    key_    = ::operator new(key_len);
    data_   = ::operator new(data_len);

    std::memcpy(key_, key, key_len);
    std::memcpy(data_, data, data_len);
}

// ---------------------------------------------------------------------------
// Destructor - HashEntry
// ---------------------------------------------------------------------------
HashEntry::~HashEntry()
{
    ::operator delete(key_);
    ::operator delete(data_);
}

// ---------------------------------------------------------------------------
// Updates Data
// ---------------------------------------------------------------------------
void
HashEntry::update_data(void *data)
{
    std::memcpy(data_, data, data_len_);
}

