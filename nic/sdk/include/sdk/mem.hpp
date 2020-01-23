//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __SDK_MEM_HPP__
#define __SDK_MEM_HPP__

#include "include/sdk/base.hpp"
#include "lib/utils/mtrack.hpp"

namespace sdk {

enum {
    SDK_MEM_ALLOC_NONE,
    SDK_MEM_ALLOC_ABST_NODES,
    SDK_MEM_ALLOC_ENTIRE_HBM_HASH_ENTRY_DATA,
    SDK_MEM_ALLOC_FLOW,
    SDK_MEM_ALLOC_HASH_HW_KEY_DEPGM,
    SDK_MEM_ALLOC_HASH_HW_KEY_INS,
    SDK_MEM_ALLOC_HASH_HW_KEY_UPD,
    SDK_MEM_ALLOC_HASH_STATS,
    SDK_MEM_ALLOC_HASH_SW_KEY_MASK_INS,
    SDK_MEM_ALLOC_HBM_HASH_BUCKETS,
    SDK_MEM_ALLOC_HBM_HASH_ENTRIES,
    SDK_MEM_ALLOC_HBM_HASH_ENTRY,
    SDK_MEM_ALLOC_HBM_HASH_ENTRY_HW_KEY,
    SDK_MEM_ALLOC_HBM_HASH_HINT_GROUP,
    SDK_MEM_ALLOC_HBM_HASH_SPINE_ENTRY,
    SDK_MEM_ALLOC_HBM_HASH_SPINE_ENTRY_HW_KEY,
    SDK_MEM_ALLOC_HBM_HASH_SPINE_ENTRY_SW_KEY,
    SDK_MEM_ALLOC_HBM_HASH_STATS,
    SDK_MEM_ALLOC_HBM_HASH_TABLE_ENTRY,
    SDK_MEM_ALLOC_ID_HASH,
    SDK_MEM_ALLOC_ID_HASH_ENTRY,
    SDK_MEM_ALLOC_ID_HASH_ENTRY_DATA,
    SDK_MEM_ALLOC_ID_HASH_ENTRY_KEY,
    SDK_MEM_ALLOC_ID_HASH_NAME,
    SDK_MEM_ALLOC_ID_HW_KEY,
    SDK_MEM_ALLOC_ID_TCAM,
    SDK_MEM_ALLOC_ID_TCAM_ENTRY,
    SDK_MEM_ALLOC_ID_TCAM_ENTRY_DATA,
    SDK_MEM_ALLOC_ID_TCAM_ENTRY_KEY,
    SDK_MEM_ALLOC_ID_TCAM_ENTRY_KEY_MASK,
    SDK_MEM_ALLOC_ID_TCAM_NAME,
    SDK_MEM_ALLOC_ID_TCAM_STATS,
    SDK_MEM_ALLOC_LIB_BITMAP,
    SDK_MEM_ALLOC_LIB_CRCFAST,
    SDK_MEM_ALLOC_LIB_DIRECTMAP,
    SDK_MEM_ALLOC_LIB_DIRECT_MAP_DATA,
    SDK_MEM_ALLOC_LIB_DIRECT_MAP_ENTRY,
    SDK_MEM_ALLOC_LIB_DIRECT_MAP_HW_DATA,
    SDK_MEM_ALLOC_LIB_DIRECT_MAP_SW_DATA,
    SDK_MEM_ALLOC_LIB_HT,
    SDK_MEM_ALLOC_LIB_INDEXER,
    SDK_MEM_ALLOC_LIB_LFQ,
    SDK_MEM_ALLOC_LIB_SHM,
    SDK_MEM_ALLOC_LIB_SLAB,
    SDK_MEM_ALLOC_LIB_THREAD,
    SDK_MEM_ALLOC_LIB_EVENT_THREAD,
    SDK_MEM_ALLOC_LIB_TWHEEL,
    SDK_MEM_ALLOC_MEM_HASH,
    SDK_MEM_ALLOC_MEM_HASH_API_CTX,
    SDK_MEM_ALLOC_MEM_HASH_HINT_TABLE,
    SDK_MEM_ALLOC_MEM_HASH_MAIN_TABLE,
    SDK_MEM_ALLOC_MEM_HASH_PROPERTIES,
    SDK_MEM_ALLOC_MEM_HASH_TABLE_BUCKETS,
    SDK_MEM_ALLOC_MEM_HASH_TABLE_ENTRIES,
    SDK_MEM_ALLOC_SLTCAM_TREE_NODES,
    SDK_MEM_ALLOC_CATALOG,
    SDK_MEM_ALLOC_DEVICE,
    SDK_MEM_ALLOC_ID_PLATFORM_MEMORY,
    SDK_MEM_ALLOC_PD,
    SDK_MEM_ALLOC_LIF_MGR,
    SDK_MEM_ALLOC_LIB_PLATFORM,
    SDK_MEM_ALLOC_LIF_QSTATE,
    SDK_MEM_ALLOC_ID_LINKMGR,
    SDK_PORT_DEBUG,
    SDK_MEM_TYPE_SLHASH_BUCKETS,
    SDK_MEM_TYPE_SLHASH,
    SDK_MEM_ALLOC_INDEX_POOL,
    SDK_MEM_ALLOC_FTL_TABLE_BUCKETS,
    SDK_MEM_ALLOC_FTL_TABLE_ENTRIES,
    SDK_MEM_ALLOC_FTL,
    SDK_MEM_ALLOC_FTL_PROPERTIES,
    SDK_MEM_ALLOC_FTL_HINT_TABLE,
    SDK_MEM_ALLOC_FTL_MAIN_TABLE,
    SDK_MEM_ALLOC_HASH_NAME,
    SDK_MEM_ALLOC_FTLV4_MAIN_TABLE,
    SDK_MEM_ALLOC_FTLV4_HINT_TABLE,
    SDK_MEM_ALLOC_FTLV4_PROPERTIES,
    SDK_MEM_ALLOC_FTLV4_TABLE_BUCKETS,
    SDK_MEM_ALLOC_FTLV4_TABLE_ENTRIES,
    SDK_MEM_ALLOC_INFRA,
    SDK_MEM_ALLOC_LFQ_MSG,
    SDK_MEM_ALLOC_LIB_DPDK_DEVICE,
    SDK_MEM_ALLOC_LIB_DPDK_BUFFER,
    SDK_MEM_ALLOC_EVENT_MGR,
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
namespace utils {
    extern sdk::utils::mem_mgr g_sdk_mem_mgr;
}
}    // namespace sdk


extern sdk::utils::mem_mgr g_sdk_mem_mgr;
#ifdef ENABLE_SDK_MTRACK
#define SDK_MALLOC(alloc_id, size)    (sdk::utils::g_sdk_mem_mgr.mtrack_alloc(static_cast<uint32_t>(alloc_id), false, size, __FUNCTION__, __LINE__))
#define SDK_CALLOC(alloc_id, size)    (sdk::utils::g_sdk_mem_mgr.mtrack_alloc(static_cast<uint32_t>(alloc_id), true, size, __FUNCTION__, __LINE__))
#define SDK_FREE(alloc_id, ptr)       (sdk::utils::g_sdk_mem_mgr.mtrack_free(static_cast<uint32_t>(alloc_id), ptr, __FUNCTION__, __LINE__))
#else
#define SDK_MALLOC(alloc_id, size)    sdk::sdk_malloc(#alloc_id, size)
#define SDK_CALLOC(alloc_id, size)    sdk::sdk_calloc(#alloc_id, size)
#define SDK_FREE(alloc_id, ptr)       ::free(ptr)
#endif

#endif    // __SDK_MEM_HPP__

