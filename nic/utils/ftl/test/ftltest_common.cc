//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#include "include/sdk/table.hpp"
#include "ftltest_common.hpp"

uint32_t g_cache_index = 0;

uint32_t
gencrc32 (bool nextindex, bool nexthint)
{
    static uint32_t index = 1;
    static uint32_t hint = 1;
    static crc32_t crc32;

    index = nextindex ? index + 1: index;
    hint = nexthint ? hint + 1: hint;

    crc32.hint = hint;
    crc32.index = index;

    return crc32.val;
}

void
fill_entry (uint32_t index, ftlv6_entry_t *entry)
{
    entry->key_metadata_proto = 17;
    entry->key_metadata_sport = index;
    entry->key_metadata_dport = index;

    entry->key_metadata_src[0] = 0x22;
    entry->key_metadata_src[12] = index >> 24 & 0xFF;
    entry->key_metadata_src[13] = index >> 16 & 0xFF;
    entry->key_metadata_src[14] = index >> 8 & 0xFF;
    entry->key_metadata_src[15] = index & 0xFF;

    entry->key_metadata_dst[0] = 0x33;
    entry->key_metadata_dst[12] = index >> 24 & 0xFF;
    entry->key_metadata_dst[13] = index >> 16 & 0xFF;
    entry->key_metadata_dst[14] = index >> 8 & 0xFF;
    entry->key_metadata_dst[15] = index & 0xFF;

    entry->session_index = index+1;
    return;
}

#define POOL_SIZE 16
sdk_table_api_params_t *
gen_entry (uint32_t index, bool with_hash, uint32_t hash_32b) {
    static ftlv6_entry_t entry[POOL_SIZE];
    static sdk_table_api_params_t params[POOL_SIZE];
    static uint32_t pidx = 0;

    fill_entry(index+1, &entry[pidx]);
    params[pidx].entry = &entry[pidx];
    params[pidx].hash_valid = with_hash;
    if (with_hash) {
        params[pidx].hash_32b = hash_32b ? hash_32b : index;
    } else {
        params[pidx].hash_32b = 0;
    }
 
    auto ret = &params[pidx];
    pidx = (pidx + 1) % POOL_SIZE;
    ret->handle.clear();
    return ret;
}

uint32_t
get_cache_count ()
{
    return g_cache_index;
}

void
reset_cache()
{
    g_cache_index = 0;
    return;
}
