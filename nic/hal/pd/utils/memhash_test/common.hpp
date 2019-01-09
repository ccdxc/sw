//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <gtest/gtest.h>
#include <stdio.h>
#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <chrono>
#include <fstream>
#include "nic/hal/pd/utils/flow/test/jenkins_spooky/spooky.h"
#include "include/sdk/base.hpp"
#include "nic/include/hal_mem.hpp"
#include <arpa/inet.h>

using sdk::table::mem_hash;
using boost::multiprecision::uint512_t;
using boost::multiprecision::uint128_t;
using namespace std::chrono;

#define MAX_HINTS       5
#define MAX_RECIRCS     8
#define NUM_INDEX_BITS  21
#define NUM_HINT_BITS   11

typedef union crc32_s {
    struct {
        uint32_t    index:NUM_INDEX_BITS;
        uint32_t    hint:NUM_HINT_BITS;
    };
    uint32_t val;
} crc32_t;

uint32_t
gencrc32(bool nextindex = true, bool nexthint = true);

void* genkey();

void* gendata();

#endif // __COMMON_HPP__
