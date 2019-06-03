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

TEST_F(TEST_CLASS_NAME, start_end)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}


TEST_F(TEST_CLASS_NAME, reserve_release)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Reserve(TEST_OP_COUNT, sdk::SDK_RET_OK,
                 TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Release(cache.count(), sdk::SDK_RET_OK,
                 TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

#if 0 
// Duplicate Reserves cannot be detected yet, as
// the key is not written to HW during reserve.
TEST_F(TEST_CLASS_NAME, dup_reserve)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Reserve(2*TEST_OP_COUNT, sdk::SDK_RET_OK,
                 TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Reserve(2*TEST_OP_COUNT, sdk::SDK_RET_NO_RESOURCE,
                 TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Release(2*TEST_OP_COUNT, sdk::SDK_RET_OK,
                 TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}
#endif

TEST_F(TEST_CLASS_NAME, reserve_dup_release)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Reserve(TEST_OP_COUNT, sdk::SDK_RET_OK,
                 TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Release(cache.count(), sdk::SDK_RET_OK,
                 TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Release(cache.count(), sdk::SDK_RET_ENTRY_NOT_FOUND,
                 TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, reserve_insert)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Reserve(TEST_OP_COUNT, sdk::SDK_RET_OK,
                 TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Insert(cache.count(), sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, DISABLED_reserve_insert_remove)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Reserve(TEST_OP_COUNT, sdk::SDK_RET_OK,
                 TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Insert(cache.count(), sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Remove(cache.count(), sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, DISABLED_reserve_dup_insert)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Reserve(TEST_OP_COUNT, sdk::SDK_RET_OK,
                 TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Insert(cache.count(), sdk::SDK_RET_OK,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Insert(cache.count(), sdk::SDK_RET_ENTRY_EXISTS,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, DISABLED_reserve_remove)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Reserve(TEST_OP_COUNT, sdk::SDK_RET_OK,
                 TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Remove(cache.count(), sdk::SDK_RET_ENTRY_NOT_FOUND,
                TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd(sdk::SDK_RET_TXN_INCOMPLETE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, dup_start)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnStart(sdk::SDK_RET_TXN_EXISTS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, dup_end)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd(sdk::SDK_RET_TXN_NOT_FOUND);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(TEST_CLASS_NAME, incomplete_end)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Reserve(TEST_OP_COUNT, sdk::SDK_RET_OK,
                 TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd(sdk::SDK_RET_TXN_INCOMPLETE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Release(cache.count(), sdk::SDK_RET_OK,
                 TEST_USE_HANDLE_OR_KEY, TEST_SET_OR_CLR_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

#if 0
TEST_F(txn, insert_collision_chain_with_handle)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = ReserveWithHash(1);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = InsertAllCached(sdk::SDK_RET_OK, true);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}
#endif

// Undef all the test parameters after the tests are instantiated.
#undef TEST_CLASS_NAME
#undef TEST_OP_COUNT
#undef TEST_USE_HANDLE_OR_KEY
#undef TEST_SET_OR_CLR_HASH
