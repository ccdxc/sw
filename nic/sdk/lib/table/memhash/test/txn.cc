//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "base.hpp"
#define BASIC_TEST_COUNT 1

class txn: public MemHashGtestBase {
};

TEST_F(txn, start)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(txn, end)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnEnd();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(txn, reserve)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Reserve(1, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(txn, reserve_collision_chain)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = ReserveWithHash(1);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(txn, insert_collision_chain_with_handle)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = ReserveWithHash(1);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = InsertAllCached(sdk::SDK_RET_OK, true);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}


TEST_F(txn, dup_start)
{
    sdk_ret_t rs;
    rs = TxnStart();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = TxnStart(sdk::SDK_RET_TXN_EXISTS);
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
}
