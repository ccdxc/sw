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

class collision: public ::testing::Test {
protected:
    mem_hash *table;

protected:
    collision() {}
    virtual ~collision() {}
    
    virtual void SetUp() {
        mem_hash_factory_params_t params = { 0 };
        params.table_id = MEM_HASH_P4TBL_ID_H5;
        params.num_hints = 5;
        params.max_recircs = MAX_RECIRCS;
        params.key2str = h5_key2str;
        params.appdata2str = h5_appdata2str;

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

TEST_F(collision, insert_full_mesh) {
    sdk_ret_t rs;
    h5_crc32_t crc32;;
    uint32_t i = 0;
    uint32_t h = 0;
    mem_hash_api_params_t params = { 0 };
    
    crc32.val = h5_gencrc32();
    h5_gen_cache_entry(&crc32, &params);
    
    rs = table->insert(&params);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    for (h = 0; h < H5_MAX_HINTS; h++) {
        crc32.hint += 1;
        for (i = 0; i < MAX_RECIRCS - 1; i++) {
            h5_gen_cache_entry(&crc32, &params);
            
            rs = table->insert(&params);
            ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        }
    }
}

TEST_F(collision, insert_update_full_mesh) {
    sdk_ret_t rs;
    h5_crc32_t crc32;
    uint32_t i = 0;
    uint32_t h = 0;
    mem_hash_api_params_t params = { 0 };
    
    crc32.val = h5_gencrc32();
    h5_gen_cache_entry(&crc32, &params);

    rs = table->insert(&params);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    for (h = 0; h < H5_MAX_HINTS; h++) {
        crc32.hint += 1;
        for (i = 0; i < MAX_RECIRCS - 1; i++) {
            h5_gen_cache_entry(&crc32, &params);
            
            rs = table->insert(&params);
            ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        }
    }

    for (i = 0; i < h5_get_cache_count(); i++) {
        h5_get_updated_cache_entry(i, &params);
       
        rs = table->update(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
}


TEST_F(collision, insert_remove_full_mesh) {
    sdk_ret_t rs;
    h5_crc32_t crc32;
    uint32_t i = 0;
    uint32_t h = 0;
    mem_hash_api_params_t params = { 0 };
    
    crc32.val = h5_gencrc32();
    h5_gen_cache_entry(&crc32, &params);

    rs = table->insert(&params);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    for (h = 0; h < H5_MAX_HINTS; h++) {
        crc32.hint += 1;
        for (i = 0; i < MAX_RECIRCS - 1; i++) {
            h5_gen_cache_entry(&crc32, &params);
            
            rs = table->insert(&params);
            ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        }
    }

    for (i = 0; i < h5_get_cache_count(); i++) {
        h5_get_cache_entry(i, &params);
        
        rs = table->remove(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
}

TEST_F(collision, insert_full_mesh_more_hashs) {
    sdk_ret_t rs;
    h5_crc32_t crc32;
    uint32_t i = 0;
    uint32_t h = 0;
    uint32_t ml = 0;
    mem_hash_api_params_t params = { 0 };
    
    crc32.val = h5_gencrc32();
    h5_gen_cache_entry(&crc32, &params);

    rs = table->insert(&params);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    for (ml = 1; ml < MAX_RECIRCS; ml++) {
        for (h = 0; h < H5_MAX_HINTS; h++) {
            crc32.hint += 1;
            for (i = 0; i < MAX_RECIRCS - ml; i++) {
                h5_gen_cache_entry(&crc32, &params);

                rs = table->insert(&params);
                ASSERT_TRUE(rs == sdk::SDK_RET_OK);
            }
        }
    }
}

TEST_F(collision, insert_update_full_mesh_more_hashs) {
    sdk_ret_t rs;
    h5_crc32_t crc32;
    uint32_t i = 0;
    uint32_t h = 0;
    uint32_t ml = 0;
    mem_hash_api_params_t params = { 0 };
    
    crc32.val = h5_gencrc32();
    h5_gen_cache_entry(&crc32, &params);

    rs = table->insert(&params);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    for (ml = 1; ml < MAX_RECIRCS; ml++) {
        for (h = 0; h < H5_MAX_HINTS; h++) {
            crc32.hint += 1;
            for (i = 0; i < MAX_RECIRCS - ml; i++) {
                h5_gen_cache_entry(&crc32, &params);

                rs = table->insert(&params);
                ASSERT_TRUE(rs == sdk::SDK_RET_OK);
            }
        }
    }

    for (i = 0; i < h5_get_cache_count(); i++) {
        h5_get_updated_cache_entry(i, &params);
        
        rs = table->update(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
}


TEST_F(collision, exm_match_chain) {
    sdk_ret_t rs;
    uint32_t i = 0;
    mem_hash_api_params_t params = { 0 };
    
    params.hash_32b = h5_gencrc32();
    params.hash_valid = true;
    
    for (i = 0; i < MAX_RECIRCS; i++) {
        params.key = h5_genkey();
        params.appdata = h5_gendata();
        
        rs = table->insert(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
}

TEST_F(collision, update_exm_match_chain) {
    sdk_ret_t rs;
    uint32_t i = 0;
    mem_hash_api_params_t params = { 0 };
    
    params.hash_32b = h5_gencrc32();
    params.hash_valid = true;
    
    for (i = 0; i < MAX_RECIRCS; i++) {
        params.key = h5_genkey();
        params.appdata = h5_gendata();
        
        rs = table->insert(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        
        params.appdata = h5_gendata();
        rs = table->update(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
}

TEST_F(collision, max_recirc) {
    sdk_ret_t rs;
    uint32_t i = 0;
    mem_hash_api_params_t params = { 0 };
    
    params.hash_32b = h5_gencrc32();
    params.hash_valid = true;
    
    for (i = 0; i < MAX_RECIRCS; i++) {
        params.key = h5_genkey();
        params.appdata = h5_gendata();
       
        rs = table->insert(&params);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }

    params.key = h5_genkey();
    params.appdata = h5_gendata();

    rs = table->insert(&params);
    ASSERT_TRUE(rs == sdk::SDK_RET_MAX_RECIRC_EXCEED);
}
