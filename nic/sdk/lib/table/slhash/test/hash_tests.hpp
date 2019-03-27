//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef TEST_CLASS_NAME
#error "TEST_CLASS_NAME must be defined before including this file."
#endif

#ifndef TEST_OP_COUNT
#error "TEST_OP_COUNT must be defined before including this file"
#endif

#ifndef TEST_USE_HANDLE_OR_KEY
#error "TEST_USE_HANDLE_OR_KEY must be defined before including this file"
#endif

#ifndef TEST_SET_OR_CLR_HASH
#error "TEST_SET_OR_CLR_HASH must be defined before including this file"
#endif


class TEST_CLASS_NAME: public SlhashGtestBase {
};

TEST_F(TEST_CLASS_NAME, insert)
{
    sdk_ret_t rs;
    rs = Insert(TEST_OP_COUNT, sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, insert_get)
{
    sdk_ret_t rs;
    rs = Insert(TEST_OP_COUNT, sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Get(cache.count(), sdk::SDK_RET_OK,
             TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, insert_update)
{
    sdk_ret_t rs;
    rs = Insert(TEST_OP_COUNT, sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Update(cache.count(), sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, insert_update_get)
{
    sdk_ret_t rs;
    rs = Insert(TEST_OP_COUNT, sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Update(cache.count(), sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Get(cache.count(), sdk::SDK_RET_OK,
             TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, repeated_update)
{
    sdk_ret_t rs;
    rs = Insert(TEST_OP_COUNT, sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    for (uint32_t j = 0; j < 5; j++) {
        rs = Update(cache.count(), sdk::SDK_RET_OK,
                    TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
}

TEST_F(TEST_CLASS_NAME, repeated_update_get)
{
    sdk_ret_t rs;
    rs = Insert(TEST_OP_COUNT, sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    for (uint32_t j = 0; j < 5; j++) {
        rs = Update(cache.count(), sdk::SDK_RET_OK,
                    TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
        ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    }
    rs = Get(cache.count(), sdk::SDK_RET_OK,
             TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, insert_duplicate) 
{
    sdk_ret_t rs;
    rs = Insert(TEST_OP_COUNT, sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Insert(cache.count(), sdk::SDK_RET_ENTRY_EXISTS,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, insert_duplicate_get) 
{
    sdk_ret_t rs;
    rs = Insert(TEST_OP_COUNT, sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Insert(cache.count(), sdk::SDK_RET_ENTRY_EXISTS,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Get(cache.count(), sdk::SDK_RET_OK,
             TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, insert_remove) 
{
    sdk_ret_t rs;
    rs = Insert(TEST_OP_COUNT, sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Remove(cache.count(), sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, insert_remove_get) 
{
    sdk_ret_t rs;
    rs = Insert(TEST_OP_COUNT, sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Remove(cache.count(), sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Get(cache.count(), sdk::SDK_RET_ENTRY_NOT_FOUND,
             TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, insert_update_remove) 
{
    sdk_ret_t rs;
    rs = Insert(TEST_OP_COUNT, sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Update(cache.count(), sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Remove(cache.count(), sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, insert_update_remove_get) 
{
    sdk_ret_t rs;
    rs = Insert(TEST_OP_COUNT, sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Update(cache.count(), sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Remove(cache.count(), sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Get(cache.count(), sdk::SDK_RET_ENTRY_NOT_FOUND,
             TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, duplicate_remove) 
{
    sdk_ret_t rs;
    rs = Insert(TEST_OP_COUNT, sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Remove(cache.count(), sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Remove(cache.count(), sdk::SDK_RET_ENTRY_NOT_FOUND,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, remove_not_found) 
{
    sdk_ret_t rs;
    rs = Remove(TEST_OP_COUNT, sdk::SDK_RET_ENTRY_NOT_FOUND,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, get_not_found) 
{
    sdk_ret_t rs;
    rs = Get(TEST_OP_COUNT, sdk::SDK_RET_ENTRY_NOT_FOUND,
             TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, iterate)
{
    sdk_ret_t rs;
    rs = Insert(TEST_OP_COUNT, sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Iterate();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

// Undef all the test parameters after the tests are instantiated.
#undef TEST_CLASS_NAME
#undef TEST_OP_COUNT
#undef TEST_USE_HANDLE_OR_KEY
#undef TEST_SET_OR_CLR_HASH
