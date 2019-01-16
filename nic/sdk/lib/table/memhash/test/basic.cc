//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "base.hpp"
#define BASIC_TEST_COUNT 64

class basic: public MemHashGtestBase {
};

TEST_F(basic, insert)
{
    sdk_ret_t rs;
    rs = Insert(BASIC_TEST_COUNT);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(basic, update)
{
    sdk_ret_t rs;
    rs = Insert(BASIC_TEST_COUNT);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = UpdateAllCached();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(basic, repeated_update)
{
    sdk_ret_t rs;
    rs = Insert(BASIC_TEST_COUNT);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    for (uint32_t j = 0; j < 5; j++) {
        rs = UpdateAllCached();
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
}

TEST_F(basic, insert_duplicate) 
{
    sdk_ret_t rs;
    rs = Insert(BASIC_TEST_COUNT);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = InsertAllCached(sdk::SDK_RET_ENTRY_EXISTS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(basic, insert_remove) 
{
    sdk_ret_t rs;
    rs = Insert(BASIC_TEST_COUNT);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = RemoveAllCached();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(basic, insert_update_remove) 
{
    sdk_ret_t rs;
    rs = Insert(BASIC_TEST_COUNT);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = UpdateAllCached();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = RemoveAllCached();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(basic, duplicate_remove) 
{
    sdk_ret_t rs;
    rs = Insert(BASIC_TEST_COUNT);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = RemoveAllCached();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = RemoveAllCached(sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(basic, remove_not_found) 
{
    sdk_ret_t rs;
    rs = Remove(BASIC_TEST_COUNT, sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}
