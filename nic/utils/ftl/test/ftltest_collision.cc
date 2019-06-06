//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "ftltest_base.hpp"

class collision: public FtlGtestBase {
};

#define MAX_COUNT     5
#define HASH_VALUE    0xDEADBEEF

TEST_F(collision, insert_full_mesh) {
    sdk_ret_t rs;
    rs = Insert(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_update_full_mesh) {
    sdk_ret_t rs;
    rs = Insert(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = Update(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_remove_full_mesh_iterate) {
    sdk_ret_t rs;
    rs = Insert(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = Iterate();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = Remove(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_full_mesh_more_hashs) {
    sdk_ret_t rs;
    rs = Insert(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

#if 0
TEST_F(collision, insert_remove_full_mesh_more_hashs) {
    sdk_ret_t rs;
    rs = InsertWithHash(MAX_MORE_LEVELS, MAX_HINTS, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = RemoveAllCached(sdk::SDK_RET_OK, false);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_update_full_mesh_more_hashs) {
    sdk_ret_t rs;
    rs = InsertWithHash(MAX_MORE_LEVELS, MAX_HINTS, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = UpdateAllCached(sdk::SDK_RET_OK, false);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, exm_match_chain) {
    sdk_ret_t rs;
    
    rs = InsertWithHash(1, 1, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, update_exm_match_chain) {
    sdk_ret_t rs;
    
    rs = InsertWithHash(1, 1, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = UpdateAllCached(sdk::SDK_RET_OK, false);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, max_recirc) {
    sdk_ret_t rs;
    rs = InsertWithHash(1, 1, MAX_RECIRCS + 1);
    ASSERT_TRUE(rs == sdk::SDK_RET_MAX_RECIRC_EXCEED);
}
#endif
