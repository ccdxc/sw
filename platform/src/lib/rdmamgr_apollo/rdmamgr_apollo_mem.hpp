//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __RDMAMGR_APOLLO_MEM_HPP__
#define __RDMAMGR_APOLLO_MEM_HPP__

#include "platform/src/lib/nicmgr/include/logger.hpp"
#include "include/sdk/base.hpp"

namespace apollo {

enum {
    RDMAMGR_APOLLO_MEM_ALLOC_NONE,
    RDMAMGR_APOLLO_MEM_ALLOC_RDMA_MGR_APOLLO
};

static inline void *
rdmamgr_malloc (const char *id_str, uint32_t size)
{
    void *ptr = malloc(size);
    NIC_LOG_DEBUG("RDMAMGR MEMORY_ALLOC for id: {}, ptr: {:#x}, size: {}",
                  id_str, (uint64_t)ptr, size);
    return ptr;
}

static inline void *
rdmamgr_calloc (const char *id_str, uint32_t size)
{
    void *ptr = calloc(1, size);
    NIC_LOG_DEBUG("RDMAMGR MEMORY_CALLOC for id: {}, ptr: {:#x}, size: {}",
                  id_str, (uint64_t)ptr, size);
    return ptr;
}

static inline void
rdmamgr_free (const char *id_str, void *ptr)
{
    NIC_LOG_DEBUG("RDMAMGR MEMORY_FREE for id: {}, ptr: {:#x}",
                  id_str, (uint64_t)ptr);
    ::free(ptr);
}

}     // namespace apollo

#define RDMAMGR_MALLOC(alloc_id, size)    apollo::rdmamgr_malloc(#alloc_id, size)
#define RDMAMGR_CALLOC(alloc_id, size)    apollo::rdmamgr_calloc(#alloc_id, size)
#define RDMAMGR_FREE(alloc_id, ptr)       apollo::rdmamgr_free(#alloc_id, ptr)

#endif    // __RDMAMGR_APOLLO_MEM_HPP__

