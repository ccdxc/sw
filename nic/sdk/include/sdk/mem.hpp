//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __SDK_MEM_HPP__
#define __SDK_MEM_HPP__

#include "sdk/base.hpp"

namespace sdk {

static inline void*
sdk_malloc (char *id_str, uint32_t size)
{
    // SDK_TRACE_DEBUG("MEMORY_ALLOC for id: %s, size: %d\n", id_str, size);
    return malloc(size);
}

static inline void*
sdk_calloc (char *id_str, uint32_t size)
{
    // SDK_TRACE_DEBUG("MEMORY_ALLOC for id: %s, size: %d\n", id_str, size);
    return calloc(1, (size));
}

#define SDK_MALLOC(alloc_id, size)    sdk_malloc(#alloc_id, size)
#define SDK_CALLOC(alloc_id, size)    sdk_calloc(#alloc_id, size)
#define SDK_FREE(alloc_id, ptr)       ::free(ptr)

}    // namespace sdk

#endif    // __SDK_MEM_HPP__

