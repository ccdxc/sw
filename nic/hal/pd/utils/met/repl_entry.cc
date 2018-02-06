#include "nic/hal/pd/utils/met/repl_entry.hpp"

using hal::pd::utils::ReplEntry;

//---------------------------------------------------------------------------
// Factory method to instantiate the class
//---------------------------------------------------------------------------
ReplEntry *
ReplEntry::factory(void *data, uint32_t data_len, uint32_t mtrack_id)
{
    void        *mem = NULL;
    ReplEntry   *re = NULL;

    mem = HAL_CALLOC(mtrack_id, sizeof(ReplEntry));
    if (!mem) {
        return NULL;
    }

    re = new (mem) ReplEntry(data, data_len);
    return re;
}

//---------------------------------------------------------------------------
// Method to free & delete the object
//---------------------------------------------------------------------------
void
ReplEntry::destroy(ReplEntry *re, uint32_t mtrack_id) 
{
    if (re) {
        re->~ReplEntry();
        HAL_FREE(mtrack_id, re);
    }
}

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
ReplEntry::ReplEntry(void *data, uint32_t data_len)
{
    data_len_   = data_len;

    // data_   = ::operator new(data_len_);
    data_ = HAL_MALLOC(HAL_MEM_ALLOC_MET_REPL_ENTRY_DATA, data_len_);

    std::memcpy(data_, data, data_len);

    prev_       = NULL;
    next_       = NULL;
}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------
ReplEntry::~ReplEntry()
{
    // ::operator delete(data_);
    HAL_FREE(HAL_MEM_ALLOC_MET_REPL_ENTRY_DATA, data_);
}

// ----------------------------------------------------------------------------
// Trace repl entry
// ----------------------------------------------------------------------------
hal_ret_t
ReplEntry::trace_repl_entry()
{
    hal_ret_t           ret = HAL_RET_OK;
    fmt::MemoryWriter   buf;

    HAL_TRACE_DEBUG("Repl_entry: data_len: {}", data_len_);
    HAL_TRACE_DEBUG("Data: ");

    uint8_t     *ptr = (uint8_t *)data_;
    for (uint32_t i = 0; i < data_len_; i++, ptr++) {
        buf.write("{:#x} ", (uint8_t)*ptr);
    }
    HAL_TRACE_DEBUG("{}", buf.c_str());

    return ret;
}

