//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cinttypes>
#include "gen/p4gen/p4/include/ftl.h"
#include "ftltest_base.hpp"

class scale: public FtlGtestBase {
};

TEST_F(scale, num1K) {
    sdk_ret_t rs;
    rs = Insert(1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Update(1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Remove(1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(scale, num4K) {
    sdk_ret_t rs;
    rs = Insert(4*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Update(4*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Remove(4*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(scale, num16K) {
    sdk_ret_t rs;
    rs = Insert(16*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Update(16*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Remove(16*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(scale, DISABLED_num256K) {
    sdk_ret_t rs;
    rs = Insert(256*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Remove(156*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(scale, DISABLED_insert1M) {
    sdk_ret_t rs;
    rs = Insert(1024*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(scale, DISABLED_insert8M) {
    sdk_ret_t rs;
    rs = Insert(8*1024*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(scale, DISABLED_insert16M) {
    sdk_ret_t rs;
    rs = Insert(16*1024*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}
