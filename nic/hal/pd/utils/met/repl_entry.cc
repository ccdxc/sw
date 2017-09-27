#include "nic/hal/pd/utils/met/repl_entry.hpp"

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
    hal_ret_t           ret = HAL_RET_OK;
    fmt::MemoryWriter   buf;

    HAL_TRACE_DEBUG("Repl_entry: data_len: {}", data_len_);
    HAL_TRACE_DEBUG("Data: ");

    uint8_t     *ptr = (uint8_t *)data_;
    for (uint32_t i = 0; i < data_len_; i++, ptr++) {
        buf.write("{:#x} ", (uint8_t)*ptr);
    }
    HAL_TRACE_DEBUG(buf.c_str());

    return ret;
}

