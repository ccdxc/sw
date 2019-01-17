//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include "include/sdk/table.hpp"
#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/sdk/lib/table/memhash/test/p4pd_mock/mem_hash_p4pd_mock.hpp"

using sdk::table::sdk_table_api_params_t;
using sdk::table::mem_hash_factory_params_t;

#define MAX_RECIRCS 8
#define MAX_MORE_LEVELS (MAX_RECIRCS - 1)

#define H5_MAX_HINTS 5
#define H5_MAX_ENTRIES 16*1024*1024
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
} h5_crc32_t;

typedef union h10_crc32_s {
    struct {
        uint32_t    index:H10_NUM_INDEX_BITS;
        uint32_t    hint:H10_NUM_HASH_BITS;
    };
    uint32_t val;
} h10_crc32_t;

typedef struct h5_entry_s {
    mem_hash_h5_key_t           key;
    mem_hash_h5_appdata_t       appdata;
    h5_crc32_t                  crc32;
} h5_entry_t;

typedef struct h10_entry_s {
    mem_hash_h10_key_t           key;
    mem_hash_h10_actiondata_t    data;
    h10_crc32_t                  crc32;
} h10_entry_t;


uint32_t h5_gencrc32(bool nextindex = true, bool nexthint = true);
void* h5_genkey();
void* h5_gendata();
void* h10_genkey();
void* h10_gendata();

uint32_t h10_gencrc32(bool nextindex = true, bool nexthint = true);
h5_entry_t* h5_get_cache_entry(uint32_t index,
                               sdk_table_api_params_t *params);
h5_entry_t * h5_get_updated_cache_entry(uint32_t index, 
                                        sdk_table_api_params_t *params);
h5_entry_t* h5_gen_cache_entry(h5_crc32_t *crc32,
                               sdk_table_api_params_t *params);
uint32_t h5_get_cache_count();
void h5_reset_cache();

h10_entry_t* h10_get_cache_entry(uint32_t index,
                                 sdk_table_api_params_t *params);
h10_entry_t* h10_gen_cache_entry(h10_crc32_t *crc32,
                                 sdk_table_api_params_t *params);
uint32_t h10_get_cache_count();
void h10_reset_cache();

char* h5_key2str(void *key);
char* h5_appdata2str(void *data);
char* h10_key2str(void *key);
char* h10_appdata2str(void *data);

#endif // __COMMON_HPP__
