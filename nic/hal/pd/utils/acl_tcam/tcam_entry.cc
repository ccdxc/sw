#include <cstring>

#include "tcam_entry.hpp"

using hal::pd::utils::TcamEntry;

//---------------------------------------------------------------------------
// Factory method to instantiate the class
//---------------------------------------------------------------------------
TcamEntry *
TcamEntry::factory(void *key, void *key_mask, uint32_t key_len,
                   void *data, uint32_t data_len, uint32_t index,
                   priority_t priority,
                   uint32_t mtrack_id)
{
    void        *mem = NULL;
    TcamEntry   *te = NULL;

    mem = HAL_CALLOC(mtrack_id, sizeof(TcamEntry));
    if (!mem) {
        return NULL;
    }

    te = new (mem) TcamEntry(key, key_mask, key_len, data, data_len, index,
                             priority);
    return te;
}

//---------------------------------------------------------------------------
// Method to free & delete the object
//---------------------------------------------------------------------------
void
TcamEntry::destroy(TcamEntry *te, uint32_t mtrack_id) 
{
    if (te) {
        te->~TcamEntry();
        HAL_FREE(mtrack_id, te);
    }
}

//---------------------------------------------------------------------------
// Constructor - TcamEntry
//---------------------------------------------------------------------------
TcamEntry::TcamEntry(void *key, void *key_mask, uint32_t key_len, 
                     void *data, uint32_t data_len, uint32_t index,
                     priority_t priority)
{
    key_len_  = key_len;
    data_len_ = data_len;
    index_    = index;
    priority_ = priority;
    refcnt_   = 0;

    key_        = ::operator new(key_len);
    key_mask_   = ::operator new(key_len);
    data_       = ::operator new(data_len);

    std::memcpy(key_, key, key_len);
    std::memcpy(key_mask_, key_mask, key_len);
    std::memcpy(data_, data, data_len);
}

//---------------------------------------------------------------------------
// Destructor - HashEntry
//---------------------------------------------------------------------------
TcamEntry::~TcamEntry()
{
    ::operator delete(key_);
    ::operator delete(key_mask_);
    ::operator delete(data_);
}

//---------------------------------------------------------------------------
// Updates Data 
//---------------------------------------------------------------------------
void
TcamEntry::update(void *key, void *key_mask, void *data, priority_t priority)
{
    priority_ = priority;
    std::memcpy(key_, key, key_len_);
    std::memcpy(key_mask_, key_mask, key_len_);
    std::memcpy(data_, data, data_len_);
}



