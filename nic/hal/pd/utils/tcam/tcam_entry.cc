#include <cstring>

#include "tcam_entry.hpp"

using hal::pd::utils::TcamEntry;

// ---------------------------------------------------------------------------
// Constructor - TcamEntry
// ---------------------------------------------------------------------------
TcamEntry::TcamEntry(void *key, void *key_mask, uint32_t key_len, 
                     void *data, uint32_t data_len, uint32_t index)
{
    key_len_  = key_len;
    data_len_ = data_len;
    index_    = index;

    key_        = ::operator new(key_len);
    key_mask_   = ::operator new(key_len);
    data_       = ::operator new(data_len);

    std::memcpy(key_, key, key_len);
    std::memcpy(key_mask_, key_mask, key_len);
    std::memcpy(data_, data, data_len);
}

// ---------------------------------------------------------------------------
// Destructor - HashEntry
// ---------------------------------------------------------------------------
TcamEntry::~TcamEntry()
{
    ::operator delete(key_);
    ::operator delete(key_mask_);
    ::operator delete(data_);
}

// ---------------------------------------------------------------------------
// Updates Data 
// ---------------------------------------------------------------------------
void
TcamEntry::update_data(void *data)
{
    std::memcpy(data_, data, data_len_);
}



