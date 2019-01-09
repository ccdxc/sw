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

class collision: public ::testing::Test {
protected:
    mem_hash *table;

protected:
    collision() {}
    virtual ~collision() {}
    
    virtual void SetUp() {
        table = mem_hash::factory(P4TBL_ID_FLOW_HASH, MAX_RECIRCS, NULL);
        assert(table);
    }
    virtual void TearDown() {
        mem_hash::destroy(table);
    }
};

TEST_F(collision, insert_full_mesh) {
    sdk_ret_t   rs;
    crc32_t     crc32;;
    uint32_t    i = 0;
    uint32_t    h = 0;
    
    crc32.val = gencrc32();

    rs = table->insert(genkey(), gendata(), crc32.val);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    for (h = 0; h < MAX_HINTS; h++) {
        crc32.hint += 1;
        for (i = 0; i < MAX_RECIRCS - 1; i++) {
            rs = table->insert(genkey(), gendata(), crc32.val);
            ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        }
    }
}

TEST_F(collision, insert_full_mesh_more_hashs) {
    sdk_ret_t   rs;
    crc32_t     crc32;;
    uint32_t    i = 0;
    uint32_t    h = 0;
    
    crc32.val = gencrc32();

    rs = table->insert(genkey(), gendata(), crc32.val);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    for (h = 0; h < 2*MAX_HINTS; h++) {
        crc32.hint += 1;
        for (i = 0; i < MAX_RECIRCS - 1; i++) {
            rs = table->insert(genkey(), gendata(), crc32.val);
            ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        }
    }
}

TEST_F(collision, exm_match_chain) {
    sdk_ret_t   rs;
    uint32_t    crc32 = gencrc32();
    uint32_t    i = 0;
    
    for (i = 0; i < MAX_RECIRCS; i++) {
        rs = table->insert(genkey(), gendata(), crc32);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
}

TEST_F(collision, max_recirc) {
    sdk_ret_t   rs;
    uint32_t    crc32 = gencrc32();
    uint32_t    i = 0;
    
    for (i = 0; i < MAX_RECIRCS; i++) {
        rs = table->insert(genkey(), gendata(), crc32);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
    rs = table->insert(genkey(), gendata(), crc32);
    ASSERT_TRUE(rs == sdk::SDK_RET_MAX_RECIRC_EXCEED);
}
