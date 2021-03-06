//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __HBM_HASH_MEM_TYPES_HPP_
#define __HBM_HASH_MEM_TYPES_HPP_

namespace sdk {
namespace table {

typedef enum {
    SDK_MEM_ALLOC_HBM_HASH_TABLE_ENTRY,
    SDK_MEM_ALLOC_HBM_HASH_SPINE_ENTRY,
    SDK_MEM_ALLOC_HBM_HASH_SPINE_ENTRY_SW_KEY,
    SDK_MEM_ALLOC_ENTIRE_HBM_HASH_ENTRY_DATA,
    SDK_MEM_ALLOC_HBM_HASH_SPINE_ENTRY_HW_KEY,
    SDK_MEM_ALLOC_FLOW,
    SDK_MEM_ALLOC_HBM_HASH_HINT_GROUP,
    SDK_MEM_ALLOC_HBM_HASH_ENTRY,
    SDK_MEM_ALLOC_HBM_HASH_BUCKETS,
    SDK_MEM_ALLOC_HBM_HASH_ENTRIES,
} hbm_hash_mem_type_t;

} // namespace table
} // namespace sdk

#endif // __HBM_HASH_MEM_TYPES_HPP_
