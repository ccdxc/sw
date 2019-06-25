//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "base.hpp"
#define BASIC_TEST_COUNT 4*1024*1024
#define COLLN_TEST_COUNT 32

class basic : public FtliteTestObject {
};

TEST_F(basic, Insert) {
    auto rs = InsertV4(BASIC_TEST_COUNT, sdk::SDK_RET_OK);
    assert(rs == sdk::SDK_RET_OK);
    rs = InsertV6(BASIC_TEST_COUNT, sdk::SDK_RET_OK);
    assert(rs == sdk::SDK_RET_OK);
}

TEST_F(basic, InsertCollisionChain) {
    auto rs = InsertCollisionV4(COLLN_TEST_COUNT, sdk::SDK_RET_OK);
    assert(rs == sdk::SDK_RET_OK);
    rs = InsertCollisionV6(COLLN_TEST_COUNT, sdk::SDK_RET_OK);
    assert(rs == sdk::SDK_RET_OK);
}
