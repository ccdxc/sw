//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __SDK_MEM_HPP__
#define __SDK_MEM_HPP__

#include "include/sdk/base.hpp"

namespace sdk {

enum {
    SDK_MEM_ALLOC_NONE,
    SDK_MEM_ALLOC_CATALOG,
    SDK_MEM_ALLOC_LIB_HT,
    SDK_MEM_ALLOC_LIB_INDEXER,
    SDK_MEM_ALLOC_LIB_SHM,
    SDK_MEM_ALLOC_LIB_SLAB,
    SDK_MEM_ALLOC_LIB_THREAD,
    SDK_MEM_ALLOC_LIB_TWHEEL,
    SDK_MEM_ALLOC_LIB_PLATFORM,
    SDK_MEM_ALLOC_LIB_DIRECTMAP,
    SDK_MEM_ALLOC_LIB_BITMAP,
};

static inline void *
sdk_malloc (const char *id_str, uint32_t size)
{
    // SDK_TRACE_DEBUG("MEMORY_ALLOC for id: %s, size: %d\n", id_str, size);
    return malloc(size);
}

static inline void *
sdk_calloc (const char *id_str, uint32_t size)
{
    // SDK_TRACE_DEBUG("MEMORY_ALLOC for id: %s, size: %d\n", id_str, size);
    return calloc(1, (size));
}

#define SDK_MALLOC(alloc_id, size)    sdk_malloc(#alloc_id, size)
#define SDK_CALLOC(alloc_id, size)    sdk_calloc(#alloc_id, size)
#define SDK_FREE(alloc_id, ptr)       ::free(ptr)

}    // namespace sdk

#endif    // __SDK_MEM_HPP__

