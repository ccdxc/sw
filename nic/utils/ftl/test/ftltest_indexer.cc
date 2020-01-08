//------------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <assert.h>
#include <string.h>
#include <cinttypes>
#include "nic/utils/ftl/ftl_base.hpp"
#include "nic/utils/ftl/ftl_indexer.hpp"

using namespace sdk::table::ftlint;

class FtlIndexerGtest: public ::testing::Test {
protected:
    indexer idx;
    uint32_t num_entries;

protected:
    FtlIndexerGtest() {
        num_entries = 256000;
    }
    virtual ~FtlIndexerGtest() {}

    virtual void SetUp() {
        idx.init(num_entries);
    }
    virtual void TearDown() {
        idx.deinit();
    }
};

TEST_F(FtlIndexerGtest, alloc) {
    sdk_ret_t ret;
    uint32_t index= 0xfff;

    ret = idx.alloc(index);
    ASSERT_TRUE(ret == sdk::SDK_RET_OK);
    ASSERT_TRUE(index == 0);

    for(auto i=0; i < num_entries; i++) {
        ret = idx.alloc(index);
        if(ret == sdk::SDK_RET_OK) {
            ASSERT_TRUE(index != 0);
        }
    }
}

TEST_F(FtlIndexerGtest, max_alloc_free_alloc) {
    sdk_ret_t ret;
    uint32_t index= 0xfff;

    for(auto i=0; i < num_entries; i++) {
        ret = idx.alloc(index);
        ASSERT_TRUE(ret == sdk::SDK_RET_OK);
        ASSERT_TRUE(index == i);
    }

    for(auto i=0; i < num_entries; i++) {
        idx.free(i);
    }

    for(auto i=0; i < num_entries; i++) {
        ret = idx.alloc(index);
        ASSERT_TRUE(ret == sdk::SDK_RET_OK);
    }
}

TEST_F(FtlIndexerGtest, overflow) {
    sdk_ret_t ret;
    uint32_t index;

    for(auto i=0; i < num_entries; i++) {
        ret = idx.alloc(index);
        ASSERT_TRUE(ret == sdk::SDK_RET_OK);
    }

    ret = idx.alloc(index);
    ASSERT_TRUE(ret == SDK_RET_NO_RESOURCE);
}
