//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "lib/table/tcam/tcam_entry.hpp"
#include <cstring>

namespace sdk {
namespace table {


//---------------------------------------------------------------------------
// returns key for hash table entry
//---------------------------------------------------------------------------
void *
tcam_entry_get_key_func (void *entry)
{
    tcam_entry_t *tcam_entry = (tcam_entry_t *)entry;

    return (void *)&tcam_entry->index;
}

//---------------------------------------------------------------------------
// compute hash 
//---------------------------------------------------------------------------
uint32_t 
tcam_entry_compute_hash_func(void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(uint32_t)) % ht_size;
}

//---------------------------------------------------------------------------
// compare keys
//---------------------------------------------------------------------------
bool 
tcam_entry_compare_key_func (void *key1, void *key2)
{
    if (*(uint32_t *)key1 == *(uint32_t *)key2) {
        return true;
    }
    return false;
}

tcam_entry_t *
tcam_entry_create(void *key, void *key_mask, uint32_t key_len,
                  void *data, uint32_t data_len, uint32_t index,
                  uint32_t priority)
{
    tcam_entry_t *te  = NULL;

    te = (tcam_entry_t *)SDK_CALLOC(SDK_MEM_ALLOC_ID_TCAM_ENTRY, 
                                    sizeof(tcam_entry_t));
    if (!te) {
        return NULL;
    }

    te->key_len  = key_len;
    te->data_len = data_len;
    te->index    = index;
    te->priority = priority;
    te->ref_cnt   = 0;

    te->key      = SDK_MALLOC(SDK_MEM_ALLOC_ID_TCAM_ENTRY_KEY, key_len);
    te->key_mask = SDK_MALLOC(SDK_MEM_ALLOC_ID_TCAM_ENTRY_KEY_MASK, key_len);
    te->data     = SDK_MALLOC(SDK_MEM_ALLOC_ID_TCAM_ENTRY_DATA, data_len);

    memcpy(te->key, key, key_len);
    memcpy(te->key_mask, key_mask, key_len);
    memcpy(te->data, data, data_len);

    return te;
}

void
tcam_entry_delete(tcam_entry_t *te)
{
    SDK_FREE(SDK_MEM_ALLOC_ID_TCAM_ENTRY_KEY, te->key);
    SDK_FREE(SDK_MEM_ALLOC_ID_TCAM_ENTRY_KEY_MASK, te->key_mask);
    SDK_FREE(SDK_MEM_ALLOC_ID_TCAM_ENTRY_DATA, te->data);
    SDK_FREE(SDK_MEM_ALLOC_ID_TCAM_ENTRY, te);
}

}   // namespace table
}   // namespace sdk
#if 0
using sdk::table::tcam_entry;
//---------------------------------------------------------------------------
// factory method to instantiate the class
//---------------------------------------------------------------------------
tcam_entry *
tcam_entry::factory(void *key, void *key_mask, uint32_t key_len,
                   void *data, uint32_t data_len, uint32_t index,
                   uint32_t priority,
                   uint32_t mtrack_id)
{
    void       *mem = NULL;
    tcam_entry *te  = NULL;

    mem = SDK_CALLOC(mtrack_id, sizeof(tcam_entry));
    if (!mem) {
        return NULL;
    }

    te = new (mem) tcam_entry(key, key_mask, key_len, data, data_len, index,
                             priority);
    return te;
}

//---------------------------------------------------------------------------
// method to free & delete the object
//---------------------------------------------------------------------------
void
tcam_entry::destroy(tcam_entry *te, uint32_t mtrack_id) 
{
    if (te) {
        te->~tcam_entry();
        SDK_FREE(mtrack_id, te);
    }
}

//---------------------------------------------------------------------------
// constructor
//---------------------------------------------------------------------------
tcam_entry::tcam_entry(void *key, void *key_mask, uint32_t key_len, 
                     void *data, uint32_t data_len, uint32_t index,
                     uint32_t priority)
{
    key_len_  = key_len;
    data_len_ = data_len;
    index_    = index;
    priority_ = priority;
    refcnt_   = 0;

    key_      = SDK_MALLOC(SDK_MEM_ALLOC_ID_TCAM_ENTRY_KEY, key_len);
    key_mask_ = SDK_MALLOC(SDK_MEM_ALLOC_ID_TCAM_ENTRY_KEY_MASK, key_len);
    data_     = SDK_MALLOC(SDK_MEM_ALLOC_ID_TCAM_ENTRY_DATA, data_len);

    memcpy(key_, key, key_len);
    memcpy(key_mask_, key_mask, key_len);
    memcpy(data_, data, data_len);
}

//---------------------------------------------------------------------------
// destructor
//---------------------------------------------------------------------------
tcam_entry::~tcam_entry()
{
    SDK_FREE(SDK_MEM_ALLOC_ID_TCAM_ENTRY_KEY, key_);
    SDK_FREE(SDK_MEM_ALLOC_ID_TCAM_ENTRY_KEY_MASK, key_mask_);
    SDK_FREE(SDK_MEM_ALLOC_ID_TCAM_ENTRY_DATA, data_);
}

//---------------------------------------------------------------------------
// updates data 
//---------------------------------------------------------------------------
void
tcam_entry::update_data(void *data)
{
    memcpy(data_, data, data_len_);
}
#endif


