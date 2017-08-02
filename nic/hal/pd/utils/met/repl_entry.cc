#include "repl_entry.hpp"

using hal::pd::utils::ReplEntry;

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------
ReplEntry::ReplEntry(void *data, uint32_t data_len)
{
    data_       = data;
    data_len_   = data_len;

    prev_       = NULL;
    next_       = NULL;
}


