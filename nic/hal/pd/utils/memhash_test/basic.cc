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

#define BASIC_TEST_COUNT 64

class basic: public ::testing::Test {
protected:
    mem_hash *table;

protected:
    basic() {}
    virtual ~basic() {}
    
    virtual void SetUp() {
        table = mem_hash::factory(P4TBL_ID_FLOW_HASH, MAX_RECIRCS, NULL);
        assert(table);
    }
    virtual void TearDown() {
        mem_hash::destroy(table);
    }
};

TEST_F(basic, insert)
{
    sdk_ret_t   rs;
    uint32_t    i = 0;
    
    for (i = 0; i < BASIC_TEST_COUNT; i++) {
        rs = table->insert(genkey(), gendata(), gencrc32());
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
}

TEST_F(basic, insert_duplicate) 
{
    sdk_ret_t   rs;
    uint32_t    i = 0;
    void        *key = NULL;
    void        *data = NULL;
    uint32_t    crc32 = 0;
    
    for (i = 0; i < BASIC_TEST_COUNT; i++) {
        key = genkey();
        data = gendata();
        crc32 = gencrc32();
        rs = table->insert(key, data, crc32);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        
        rs = table->insert(key, data, crc32);
        ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_EXISTS);
    }
}

TEST_F(basic, insert_remove) 
{
    sdk_ret_t   rs;
    uint32_t    i = 0;
    void        *key = NULL;
    void        *data = NULL;
    uint32_t    crc32 = 0;
    
    for (i = 0; i < BASIC_TEST_COUNT; i++) {
        key = genkey();
        data = gendata();
        crc32 = gencrc32();
        rs = table->insert(key, data, crc32);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        
        rs = table->remove(key, crc32);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
}

TEST_F(basic, duplicate_remove) 
{
    sdk_ret_t   rs;
    uint32_t    i = 0;
    void        *key = NULL;
    void        *data = NULL;
    uint32_t    crc32 = 0;
    
    for (i = 0; i < BASIC_TEST_COUNT; i++) {
        key = genkey();
        data = gendata();
        crc32 = gencrc32();
        rs = table->insert(key, data, crc32);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        
        rs = table->remove(key, crc32);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        
        rs = table->remove(key, crc32);
        ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_NOT_FOUND);
    }
}


TEST_F(basic, remove_not_found) 
{
    sdk_ret_t   rs;
    uint32_t    i = 0;
    
    for (i = 0; i < BASIC_TEST_COUNT; i++) {
        rs = table->remove(genkey(), gencrc32());
        ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_NOT_FOUND);
    }
}
