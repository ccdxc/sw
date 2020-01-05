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
