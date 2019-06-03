//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "base.hpp"

#define TXN_TEST_COUNT SLTCAM_MOCK_TABLE_SIZE

class txn: public SltcamGtestBase {
};

TEST_F(txn, start_end)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}


TEST_F(txn, reserve_release)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Reserve(TXN_TEST_COUNT, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = ReleaseAllCached(sdk::SDK_RET_OK, true);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(txn, dup_reserve)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Reserve(TXN_TEST_COUNT, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Reserve(TXN_TEST_COUNT, sdk::SDK_RET_NO_RESOURCE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = ReleaseAllCached(sdk::SDK_RET_OK, true);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(txn, reserve_dup_release)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Reserve(TXN_TEST_COUNT, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = ReleaseAllCached(sdk::SDK_RET_OK, true);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = ReleaseAllCached(sdk::SDK_RET_ENTRY_NOT_FOUND, true);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}


TEST_F(txn, reserve_insert)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Reserve(TXN_TEST_COUNT, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = InsertAllCached(sdk::SDK_RET_OK, true);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(txn, reserve_insert_remove)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Reserve(TXN_TEST_COUNT, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = InsertAllCached(sdk::SDK_RET_OK, true);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(txn, reserve_dup_insert)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Reserve(TXN_TEST_COUNT, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = InsertAllCached(sdk::SDK_RET_OK, true);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = InsertAllCached(sdk::SDK_RET_ENTRY_EXISTS, true);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(txn, reserve_remove)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Reserve(TXN_TEST_COUNT, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = RemoveAllCached(sdk::SDK_RET_ENTRY_NOT_FOUND, true);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd(sdk::SDK_RET_TXN_INCOMPLETE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(txn, dup_start)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnStart(sdk::SDK_RET_TXN_EXISTS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(txn, dup_end)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd(sdk::SDK_RET_TXN_NOT_FOUND);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(txn, incomplete_end)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Reserve(1, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd(sdk::SDK_RET_TXN_INCOMPLETE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = ReleaseAllCached(sdk::SDK_RET_OK, true);
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
