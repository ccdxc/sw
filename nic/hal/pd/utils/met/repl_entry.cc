#include "repl_entry.hpp"

using hal::pd::utils::ReplEntry;

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------
ReplEntry::ReplEntry(void *data, uint32_t data_len)
{
    data_len_   = data_len;

    data_   = ::operator new(data_len_);

    std::memcpy(data_, data, data_len);

    prev_       = NULL;
    next_       = NULL;
}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------
ReplEntry::~ReplEntry()
{
    ::operator delete(data_);
}

// ----------------------------------------------------------------------------
// Trace repl entry
// ----------------------------------------------------------------------------
hal_ret_t
ReplEntry::trace_repl_entry()
{
    hal_ret_t   ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("Repl_entry: data_len: {}", data_len_);
    HAL_TRACE_DEBUG("Data: ");

    uint8_t     *ptr = NULL;
    ptr = (uint8_t *)data_;
    for (uint32_t i = 0; i < data_len_; i++, ptr++) {
        HAL_TRACE_DEBUG("{:#x} ", (*ptr));
    }

    return ret;
}

