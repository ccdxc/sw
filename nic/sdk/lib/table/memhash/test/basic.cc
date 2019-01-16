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
using sdk::table::mem_hash_api_params_t;
using sdk::table::mem_hash_factory_params_t;

#define BASIC_TEST_COUNT 64

class basic: public ::testing::Test {
protected:
    mem_hash *table;

protected:
    basic() {}
    virtual ~basic() {}
    
    virtual void SetUp() {
        mem_hash_factory_params_t   params = { 0 };
        params.table_id = MEM_HASH_P4TBL_ID_H5;
        params.num_hints = 5;
        params.max_recircs = MAX_RECIRCS;
        params.key2str = NULL; //h5_key2str;
        params.appdata2str = NULL; //h5_appdata2str;
        table = mem_hash::factory(&params);
        assert(table);
        mem_hash_mock_init();
    }
    virtual void TearDown() {
        mem_hash::destroy(table);
        h5_reset_cache();
        mem_hash_mock_cleanup();
    }
};

TEST_F(basic, insert)
{
    sdk_ret_t rs;
    uint32_t i = 0;
    mem_hash_api_params_t params = { 0 };
    
    for (i = 0; i < BASIC_TEST_COUNT; i++) {
        params.key = h5_genkey();
        params.appdata = h5_gendata();
        params.hash_32b = h5_gencrc32();
        params.hash_valid = true;

        rs = table->insert(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
}

TEST_F(basic, update)
{
    sdk_ret_t rs;
    uint32_t i = 0;
    mem_hash_api_params_t params = { 0 };
    
    for (i = 0; i < BASIC_TEST_COUNT; i++) {
        params.key = h5_genkey();
        params.appdata = h5_gendata();
        params.hash_32b = h5_gencrc32();
        params.hash_valid = true;

        rs = table->insert(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        
        params.appdata = h5_gendata();
        rs = table->update(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
}

TEST_F(basic, repeated_update)
{
    sdk_ret_t rs;
    uint32_t i = 0;
    mem_hash_api_params_t params = { 0 };
    
    for (i = 0; i < BASIC_TEST_COUNT; i++) {
        params.key = h5_genkey();
        params.appdata = h5_gendata();
        params.hash_32b = h5_gencrc32();
        params.hash_valid = true;

        rs = table->insert(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        
        for (uint32_t j = 0; j < 5; j++) {
            params.appdata = h5_gendata();
            rs = table->update(&params);
            ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        }
    }
}


TEST_F(basic, insert_duplicate) 
{
    sdk_ret_t rs;
    uint32_t i = 0;
    mem_hash_api_params_t params = { 0 };
    
    for (i = 0; i < BASIC_TEST_COUNT; i++) {
        params.key = h5_genkey();
        params.appdata = h5_gendata();
        params.hash_32b = h5_gencrc32();
        params.hash_valid = true;

        rs = table->insert(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        
        rs = table->insert(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_EXISTS);
    }
}

TEST_F(basic, insert_remove) 
{
    sdk_ret_t rs;
    uint32_t i = 0;
    mem_hash_api_params_t params = { 0 };
    
    for (i = 0; i < BASIC_TEST_COUNT; i++) {
        params.key = h5_genkey();
        params.appdata = h5_gendata();
        params.hash_32b = h5_gencrc32();
        params.hash_valid = true;
        
        rs = table->insert(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        
        rs = table->remove(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
}

TEST_F(basic, insert_update_remove) 
{
    sdk_ret_t rs;
    uint32_t i = 0;
    mem_hash_api_params_t params = { 0 };
    
    for (i = 0; i < BASIC_TEST_COUNT; i++) {
        params.key = h5_genkey();
        params.appdata = h5_gendata();
        params.hash_32b = h5_gencrc32();
        params.hash_valid = true;
        
        rs = table->insert(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        
        params.appdata = h5_gendata();
        rs = table->update(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);

        rs = table->remove(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
}


TEST_F(basic, duplicate_remove) 
{
    sdk_ret_t rs;
    uint32_t i = 0;
    mem_hash_api_params_t params = { 0 };
    
    for (i = 0; i < BASIC_TEST_COUNT; i++) {
        params.key = h5_genkey();
        params.appdata = h5_gendata();
        params.hash_32b = h5_gencrc32();
        params.hash_valid = true;
        
        rs = table->insert(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        
        rs = table->remove(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        
        rs = table->remove(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_NOT_FOUND);
    }
}


TEST_F(basic, remove_not_found) 
{
    sdk_ret_t rs;
    uint32_t i = 0;
    mem_hash_api_params_t params = { 0 };
    
    for (i = 0; i < BASIC_TEST_COUNT; i++) {
        params.key = h5_genkey();
        params.appdata = h5_gendata();
        params.hash_32b = h5_gencrc32();
        params.hash_valid = true;

        rs = table->remove(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_ENTRY_NOT_FOUND);
    }
}
