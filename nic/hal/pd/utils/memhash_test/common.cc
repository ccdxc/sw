//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
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

#include "common.hpp"

using sdk::table::mem_hash;
using boost::multiprecision::uint512_t;
using boost::multiprecision::uint128_t;
using namespace std::chrono;

uint32_t
gencrc32(bool nextindex, bool nexthint)
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

void*
genkey()
{
    static flow_hash_swkey key;
    static uint16_t sport = 1;
    static uint16_t dport = 1;
    static uint32_t src = 1;
    static uint32_t dst = 1;
    static uint8_t proto = 1;
    static uint16_t vrf = 1;
    char srcstr[64];
    char dststr[64];

    key.flow_lkp_metadata_lkp_vrf = vrf++;
    key.flow_lkp_metadata_lkp_proto = proto++;
    key.flow_lkp_metadata_lkp_sport = sport++;
    key.flow_lkp_metadata_lkp_dport = dport++;
    src++;
    sprintf(srcstr, "2000::%04d:%04d", (src >> 16) & 0xFFFF, src & 0xFFFF);
    inet_pton(AF_INET6, srcstr, key.flow_lkp_metadata_lkp_src);

    dst++;
    sprintf(dststr, "3000::%04d:%04d", (dst >> 16) & 0xFFFF, dst & 0xFFFF);
    inet_pton(AF_INET6, dststr, key.flow_lkp_metadata_lkp_dst);
    
    key.flow_lkp_metadata_lkp_dir = 1;
    key.flow_lkp_metadata_lkp_inst = 1;
    key.flow_lkp_metadata_lkp_type = 1;

    return (void *)&key;
}

void*
gendata()
{
    static uint32_t flow_index = 1;
    static flow_hash_actiondata_t data;

    data.action_id = 0;
    data.action_u.flow_hash_flow_hash_info.export_en = 1;
    data.action_u.flow_hash_flow_hash_info.flow_index = flow_index++;

    return (void *)&data;
}
