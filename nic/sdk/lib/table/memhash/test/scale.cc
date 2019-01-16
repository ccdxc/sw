//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "base.hpp"

class scale: public MemHashGtestBase {
};

TEST_F(scale, num1K) {
    sdk_ret_t rs;
    rs = Insert(1024);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = UpdateAllCached();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = RemoveAllCached();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(scale, num4K) {
    sdk_ret_t rs;
    rs = Insert(4*1024);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = UpdateAllCached();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = RemoveAllCached();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(scale, num16K) {
    sdk_ret_t rs;
    rs = Insert(16*1024);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = UpdateAllCached();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = RemoveAllCached();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(scale, DISABLED_num256K) {
    sdk_ret_t rs;
    rs = Insert(256*1024);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = RemoveAllCached();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(scale, DISABLED_num1M) {
    sdk_ret_t rs;
    rs = Insert(1024*1024);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = RemoveAllCached();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}
