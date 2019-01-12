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

class collision: public ::testing::Test {
protected:
    mem_hash *table;

protected:
    collision() {}
    virtual ~collision() {}
    
    virtual void SetUp() {
        table = mem_hash::factory(MEM_HASH_P4TBL_ID_H5, MAX_RECIRCS, NULL);
        assert(table);
    }
    virtual void TearDown() {
        mem_hash::destroy(table);
        h5_reset_cache();
    }
};

TEST_F(collision, insert_full_mesh) {
    sdk_ret_t   rs;
    h5_crc32_t  crc32;;
    uint32_t    i = 0;
    uint32_t    h = 0;
    h5_entry_t  *entry = NULL;
    
    crc32.val = h5_gencrc32();
    entry = h5_gen_cache_entry(&crc32);

    rs = table->insert(&entry->key, &entry->data, entry->crc32.val);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    for (h = 0; h < H5_MAX_HINTS; h++) {
        crc32.hint += 1;
        for (i = 0; i < MAX_RECIRCS - 1; i++) {
            entry = h5_gen_cache_entry(&crc32);
            rs = table->insert(&entry->key, &entry->data, entry->crc32.val);
            ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        }
    }
}

TEST_F(collision, insert_remove_full_mesh) {
    sdk_ret_t   rs;
    h5_crc32_t  crc32;
    uint32_t    i = 0;
    uint32_t    h = 0;
    h5_entry_t  *entry = NULL;
    
    crc32.val = h5_gencrc32();
    entry = h5_gen_cache_entry(&crc32);

    rs = table->insert(&entry->key, &entry->data, entry->crc32.val);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    for (h = 0; h < H5_MAX_HINTS; h++) {
        crc32.hint += 1;
        for (i = 0; i < MAX_RECIRCS - 1; i++) {
            entry = h5_gen_cache_entry(&crc32);
            rs = table->insert(&entry->key, &entry->data, entry->crc32.val);
            ASSERT_TRUE(rs == sdk::SDK_RET_OK);
        }
    }

    for (i = 0; i < h5_get_cache_count(); i++) {
        entry = h5_get_cache_entry(i);
        rs = table->remove(&entry->key, entry->crc32.val);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
}

TEST_F(collision, insert_full_mesh_more_hashs) {
    sdk_ret_t   rs;
    h5_crc32_t  crc32;
    uint32_t    i = 0;
    uint32_t    h = 0;
    uint32_t    ml = 0;
    h5_entry_t  *entry = NULL;
    
    crc32.val = h5_gencrc32();
    entry = h5_gen_cache_entry(&crc32);

    rs = table->insert(&entry->key, &entry->data, entry->crc32.val);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    for (ml = 1; ml < MAX_RECIRCS; ml++) {
        for (h = 0; h < H5_MAX_HINTS; h++) {
            crc32.hint += 1;
            for (i = 0; i < MAX_RECIRCS - ml; i++) {
                entry = h5_gen_cache_entry(&crc32);
                rs = table->insert(&entry->key, &entry->data, entry->crc32.val);
                ASSERT_TRUE(rs == sdk::SDK_RET_OK);
            }
        }
    }
}

TEST_F(collision, exm_match_chain) {
    sdk_ret_t   rs;
    uint32_t    crc32 = h5_gencrc32();
    uint32_t    i = 0;
    
    for (i = 0; i < MAX_RECIRCS; i++) {
        rs = table->insert(h5_genkey(), h5_gendata(), crc32);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
}

TEST_F(collision, max_recirc) {
    sdk_ret_t   rs;
    uint32_t    crc32 = h5_gencrc32();
    uint32_t    i = 0;
    
    for (i = 0; i < MAX_RECIRCS; i++) {
        rs = table->insert(h5_genkey(), h5_gendata(), crc32);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
    rs = table->insert(h5_genkey(), h5_gendata(), crc32);
    ASSERT_TRUE(rs == sdk::SDK_RET_MAX_RECIRC_EXCEED);
}
