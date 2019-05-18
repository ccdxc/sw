//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include "include/sdk/table.hpp"
#include "nic/utils/ftlv4/ftlv4.hpp"
#include "nic/utils/ftlv4/test/p4pd_mock/ftlv4_p4pd_mock.hpp"

using sdk::table::sdk_table_api_params_t;

#define MAX_RECIRCS 8
#define MAX_MORE_LEVELS (MAX_RECIRCS - 1)

#define H5_MAX_HINTS 5
#define H5_MAX_ENTRIES 2*1024*1024
#define H5_NUM_INDEX_BITS 24
#define H5_NUM_HASH_BITS 8

#define H10_MAX_HINTS 10
#define H10_MAX_ENTRIES 256*1024
#define H10_NUM_INDEX_BITS 18
#define H10_NUM_HASH_BITS 14

typedef union h5_crc32_s {
    struct {
        uint32_t    index:H5_NUM_INDEX_BITS;
        uint32_t    hint:H5_NUM_HASH_BITS;
    };
    uint32_t val;
} crc32_t;

uint32_t h5_gencrc32(bool nextindex = true, bool nexthint = true);
char* h5_key2str(void *key);
char* h5_appdata2str(void *data);


uint32_t get_cache_count();
void reset_cache();
sdk_table_api_params_t *gen_entry (uint32_t index = 0,
                                   bool with_hash = false);

#endif // __COMMON_HPP__
