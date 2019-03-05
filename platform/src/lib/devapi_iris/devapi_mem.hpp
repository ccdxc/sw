//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __DEVAPI_MEM_HPP__
#define __DEVAPI_MEM_HPP__

#include "include/sdk/base.hpp"

namespace iris {

enum {
    DEVAPI_MEM_ALLOC_NONE,
    DEVAPI_MEM_ALLOC_DEVAPI_IRIS,
    DEVAPI_MEM_ALLOC_UPLINK,
    DEVAPI_MEM_ALLOC_VRF,
    DEVAPI_MEM_ALLOC_L2SEG,
    DEVAPI_MEM_ALLOC_EP,
    DEVAPI_MEM_ALLOC_ENIC,
    DEVAPI_MEM_ALLOC_MCAST,
    DEVAPI_MEM_ALLOC_FILTER,
    DEVAPI_MEM_ALLOC_LIF,
    DEVAPI_MEM_L2SEG_INFO,
    DEVAPI_MEM_ALLOC_DEVAPI_HAL_GRPC,
};

static inline void *
devapi_malloc (const char *id_str, uint32_t size)
{
    // DEVAPI_TRACE_DEBUG("MEMORY_ALLOC for id: %s, size: %d\n", id_str, size);
    return malloc(size);
}

static inline void *
devapi_calloc (const char *id_str, uint32_t size)
{
    // DEVAPI_TRACE_DEBUG("MEMORY_ALLOC for id: %s, size: %d\n", id_str, size);
    return calloc(1, (size));
}

} // namespace iris

#define DEVAPI_MALLOC(alloc_id, size)    iris::devapi_malloc(#alloc_id, size)
#define DEVAPI_CALLOC(alloc_id, size)    iris::devapi_calloc(#alloc_id, size)
#define DEVAPI_FREE(alloc_id, ptr)       ::free(ptr)

#endif    // __DEVAPI_MEM_HPP__

