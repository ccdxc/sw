//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "base.hpp"

class collision: public MemHashGtestBase {
};

TEST_F(collision, insert_full_mesh) {
    sdk_ret_t rs;
    rs = InsertWithHash(1, H5_MAX_HINTS, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_update_full_mesh) {
    sdk_ret_t rs;
    rs = InsertWithHash(1, H5_MAX_HINTS, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = UpdateAllCached();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}


TEST_F(collision, insert_remove_full_mesh) {
    sdk_ret_t rs;
    rs = InsertWithHash(1, H5_MAX_HINTS, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = RemoveAllCached();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_full_mesh_more_hashs) {
    sdk_ret_t rs;
    rs = InsertWithHash(MAX_MORE_LEVELS, H5_MAX_HINTS, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_remove_full_mesh_more_hashs) {
    sdk_ret_t rs;
    rs = InsertWithHash(MAX_MORE_LEVELS, H5_MAX_HINTS, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = RemoveAllCached();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_update_full_mesh_more_hashs) {
    sdk_ret_t rs;
    rs = InsertWithHash(MAX_MORE_LEVELS, H5_MAX_HINTS, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = UpdateAllCached();
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

    rs = UpdateAllCached();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, max_recirc) {
    sdk_ret_t rs;
    rs = InsertWithHash(1, 1, MAX_RECIRCS + 1);
    ASSERT_TRUE(rs == sdk::SDK_RET_MAX_RECIRC_EXCEED);
}
