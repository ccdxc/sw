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

#define MHTEST_CHECK_RETURN(_exp, _ret) if (!(_exp)) return (_ret)

class collision: public ::testing::Test {
protected:
    mem_hash *table;
    uint32_t num_insert;
    uint32_t num_remove;
    uint32_t num_update;

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

        num_insert = 0;
        num_remove = 0;
        num_update = 0;
    }
    virtual void TearDown() {
        mem_hash::destroy(table);
        h5_reset_cache();
        ValidateStats();
        mem_hash_mock_cleanup();
    }

    sdk_ret_t InsertAll(uint32_t more_levels, uint32_t max_hints,
                        uint32_t max_recircs) {
        sdk_ret_t rs;
        h5_crc32_t crc32;
        uint32_t i = 0;
        uint32_t h = 0;
        uint32_t ml = 0;
        mem_hash_api_params_t params = { 0 };
        
        crc32.val = h5_gencrc32();
        h5_gen_cache_entry(&crc32, &params);

        rs = table->insert(&params);
        MHTEST_CHECK_RETURN(rs == sdk::SDK_RET_OK, rs);

        for (ml = 1; ml < more_levels+1; ml++) {
            for (h = 0; h < max_hints; h++) {
                crc32.hint += 1;
                for (i = 0; i < max_recircs - ml; i++) {
                    h5_gen_cache_entry(&crc32, &params);

                    rs = table->insert(&params);
                    MHTEST_CHECK_RETURN(rs == sdk::SDK_RET_OK, rs);;
                }
            }
        }
        return SDK_RET_OK;
    }

    sdk_ret_t UpdateAll() {
        sdk_ret_t rs;
        uint32_t i = 0;
        mem_hash_api_params_t params = { 0 };

        for (i = 0; i < h5_get_cache_count(); i++) {
            h5_get_updated_cache_entry(i, &params);
            rs = table->update(&params);
            MHTEST_CHECK_RETURN(rs == sdk::SDK_RET_OK, rs);
        }
        return SDK_RET_OK;
    }

    sdk_ret_t RemoveAll() {
        sdk_ret_t rs;
        uint32_t i = 0;
        mem_hash_api_params_t params = { 0 };

        for (i = 0; i < h5_get_cache_count(); i++) {
            h5_get_cache_entry(i, &params);
            
            rs = table->remove(&params);
            MHTEST_CHECK_RETURN(rs == sdk::SDK_RET_OK, rs);
        }
        return SDK_RET_OK;
    }

    sdk_ret_t ValidateStats() {
        uint32_t mtable_count;
        uint32_t htable_count;

        mtable_count = mem_hash_mock_get_valid_count(MEM_HASH_P4TBL_ID_H5);
        htable_count = mem_hash_mock_get_valid_count(MEM_HASH_P4TBL_ID_H5_OHASH);

        printf("Main Table Count = %d\n", mtable_count);
        printf("Hint Table Count = %d\n", htable_count);

        return SDK_RET_OK;
    }
};

TEST_F(collision, insert_full_mesh) {
    sdk_ret_t rs;
    rs = InsertAll(1, H5_MAX_HINTS, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_update_full_mesh) {
    sdk_ret_t rs;
    rs = InsertAll(1, H5_MAX_HINTS, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = UpdateAll();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}


TEST_F(collision, insert_remove_full_mesh) {
    sdk_ret_t rs;
    rs = InsertAll(1, H5_MAX_HINTS, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = RemoveAll();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_full_mesh_more_hashs) {
    sdk_ret_t rs;
    rs = InsertAll(MAX_MORE_LEVELS, H5_MAX_HINTS, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_remove_full_mesh_more_hashs) {
    sdk_ret_t rs;
    rs = InsertAll(MAX_MORE_LEVELS, H5_MAX_HINTS, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = RemoveAll();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_update_full_mesh_more_hashs) {
    sdk_ret_t rs;
    rs = InsertAll(MAX_MORE_LEVELS, H5_MAX_HINTS, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = UpdateAll();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, exm_match_chain) {
    sdk_ret_t rs;
    
    rs = InsertAll(1, 1, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, update_exm_match_chain) {
    sdk_ret_t rs;
    
    rs = InsertAll(1, 1, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = UpdateAll();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, max_recirc) {
    sdk_ret_t rs;
    rs = InsertAll(1, 1, MAX_RECIRCS + 1);
    ASSERT_TRUE(rs == sdk::SDK_RET_MAX_RECIRC_EXCEED);
}
