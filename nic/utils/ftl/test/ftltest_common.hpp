//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cinttypes>
#include "include/sdk/table.hpp"
#include "gen/p4gen/p4/include/ftl.h"
#include "nic/utils/ftl/ftl_base.hpp"
#include "p4pd_mock/ftl_p4pd_mock.hpp"

using sdk::table::sdk_table_api_params_t;

#define MAX_RECIRCS 8
#define MAX_MORE_LEVELS (MAX_RECIRCS - 1)

#define NUM_INDEX_BITS 23
#define NUM_HASH_BITS 9

#define POOL_SIZE 16

typedef union crc32_s {
    struct {
        uint32_t    index:NUM_INDEX_BITS;
        uint32_t    hint:NUM_HASH_BITS;
    };
    uint32_t val;
} crc32_t;

uint32_t
gencrc32(bool nextindex = true, bool nexthint = true);
char* key2str(void *key);
char* appdata2str(void *data);
uint32_t get_cache_count();
void reset_cache();
sdk_table_api_params_t *gen_entry (uint32_t index = 0,
                                   bool with_hash = false,
                                   uint32_t hash_32b = 0);

#endif // __COMMON_HPP__
