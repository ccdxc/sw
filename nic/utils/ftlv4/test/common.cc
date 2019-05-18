//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#include "include/sdk/table.hpp"
#include "nic/utils/ftlv4/ftlv4_structs.hpp"
#include "common.hpp"

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
fill_entry (uint32_t index, ftlv4_entry_t *entry)
{
    entry->proto = 17;
    entry->sport = index;
    entry->dport = index;
    entry->src = (0x20 << 24) | index;
    entry->dst = (0x30 << 24) | index;
    entry->session_index = index+1;
    return;
}

#define POOL_SIZE 16
sdk_table_api_params_t *
gen_entry (uint32_t index, bool with_hash) {
    static ftlv4_entry_t entry[POOL_SIZE];
    static sdk_table_api_params_t params[POOL_SIZE];
    static uint32_t pidx = 0;

    fill_entry(index+1, &entry[pidx]);
    params[pidx].entry = &entry[pidx];
    params[pidx].hash_valid = with_hash;
    if (with_hash) {
        params[pidx].hash_32b = index;
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
