//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include <gtest/gtest.h>
#include <stdio.h>

#include "include/sdk/base.hpp"
#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/sdk/lib/table/memhash/test/p4pd_mock/mem_hash_p4pd_mock.hpp"

#include "common.hpp"

using sdk::table::mem_hash;

#define BASIC_TEST_COUNT 64

class basic: public ::testing::Test {
protected:
    mem_hash *table;

protected:
    basic() {}
    virtual ~basic() {}
    
    virtual void SetUp() {
        table = mem_hash::factory(MEM_HASH_P4TBL_ID_H5, MAX_RECIRCS, 
                                  NULL, h5_key2str, h5_data2str);
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
        rs = table->insert(h5_genkey(), h5_gendata(), h5_gencrc32());
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
        key = h5_genkey();
        data = h5_gendata();
        crc32 = h5_gencrc32();
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
        key = h5_genkey();
        data = h5_gendata();
        crc32 = h5_gencrc32();
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
        key = h5_genkey();
        data = h5_gendata();
        crc32 = h5_gencrc32();
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
        rs = table->remove(h5_genkey(), h5_gencrc32());
        ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_NOT_FOUND);
    }
}
