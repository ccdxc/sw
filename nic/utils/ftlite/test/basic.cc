//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "base.hpp"
#ifdef SIM
#define BASIC_TEST_COUNT 1024
#else
#define BASIC_TEST_COUNT 8*1024*1024
#endif

void
basic_insert_ipv4(FtliteTestObject *obj)
{
    auto rs = obj->Insert(BASIC_TEST_COUNT, sdk::SDK_RET_OK, 1, false);
    assert(rs == sdk::SDK_RET_OK);
}

void
colln_insert_ipv4(FtliteTestObject *obj)
{
    auto rs = obj->Insert(BASIC_TEST_COUNT/2, sdk::SDK_RET_OK, 1, true);
    assert(rs == sdk::SDK_RET_OK);
}

void
basic_insert_ipv6(FtliteTestObject *obj)
{
    auto rs = obj->Insert(BASIC_TEST_COUNT, sdk::SDK_RET_OK, 2, false);
    assert(rs == sdk::SDK_RET_OK);
}

void
colln_insert_ipv6(FtliteTestObject *obj)
{
    auto rs = obj->Insert(BASIC_TEST_COUNT/2, sdk::SDK_RET_OK, 2, true);
    assert(rs == sdk::SDK_RET_OK);
}

void
run_basic_tests() {
    FtliteTestObject testobj;
    testobj.SetUp("basic", "all");
    RUN_TEST(basic_insert_ipv4);
    RUN_TEST(colln_insert_ipv4);
    RUN_TEST(basic_insert_ipv6);
    RUN_TEST(colln_insert_ipv6);
    testobj.TearDown("basic", "all");
}

