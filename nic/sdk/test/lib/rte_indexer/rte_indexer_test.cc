//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// Test Cases to verify rte_indexer library
//------------------------------------------------------------------------------

#include <iostream>
#include <stdio.h>
#include "gtest/gtest.h"
#include "lib/rte_indexer/rte_indexer.hpp"

using sdk::lib::rte_indexer;

//------------------------------------------------------------------------------
// RTE Indexer Test Class
//------------------------------------------------------------------------------
class rte_indexer_test : public ::testing::Test {
protected:
  rte_indexer_test() {
  }

  virtual ~rte_indexer_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};

//----------------------------------------------------------------------------
// Test 1:
//
// Summary:
// --------
//  - Tests the basic case of allocation and free of an index
//----------------------------------------------------------------------------
TEST_F(rte_indexer_test, test1) {
    uint32_t i;

    // Instantiate the indexer
    rte_indexer  *ind = rte_indexer::factory(100);

    // Allocate index
    sdk_ret_t rs  = ind->alloc(&i);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Free index
    rs = ind->free(i);
    ASSERT_TRUE(rs == SDK_RET_OK);
    rte_indexer::destroy(ind);
}

TEST_F(rte_indexer_test, scale_test1M_thread_unsafe) {
    uint32_t i;

    // Instantiate the indexer
    rte_indexer  *ind = rte_indexer::factory(1024*1024, false);

    for (auto c = 0; c < 1024*1024; c++) {
        // Allocate index
        sdk_ret_t rs  = ind->alloc(&i);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }
    rte_indexer::destroy(ind);
}

TEST_F(rte_indexer_test, scale_test1M_thread_safe) {
    uint32_t i;

    // Instantiate the indexer
    rte_indexer  *ind = rte_indexer::factory(1024*1024);

    for (auto c = 0; c < 1024*1024; c++) {
        // Allocate index
        sdk_ret_t rs  = ind->alloc(&i);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }
    rte_indexer::destroy(ind);
}

//----------------------------------------------------------------------------
// Test 2:
//
// Summary:
// --------
//  - Tests the case of multiple allocations and frees
//----------------------------------------------------------------------------
TEST_F(rte_indexer_test, test2) {
    sdk_ret_t rs = SDK_RET_OK;
    uint32_t i,j;

    // Instantiate the indexer
    rte_indexer *ind1 = rte_indexer::factory(100);

    // Allocate index
    rs  = ind1->alloc(&i);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Allocate index
    rs  = ind1->alloc(&j);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Free index
    rs = ind1->free(i);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Free index
    rs = ind1->free(j);
    ASSERT_TRUE(rs == SDK_RET_OK);
    rte_indexer::destroy(ind1);
}

//----------------------------------------------------------------------------
// Test 3:
//
// Summary:
// --------
//  - Tests the interleaved multiple allocations and frees
//----------------------------------------------------------------------------
TEST_F(rte_indexer_test, test3) {
    sdk_ret_t rs = SDK_RET_OK;
    uint32_t i,j;

    // Instantiate the indexer
    rte_indexer *ind1 = rte_indexer::factory(100);

    // Allocate index
    rs  = ind1->alloc(&i);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Free index
    rs = ind1->free(i);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Allocate index
    rs  = ind1->alloc(&j);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Free index
    rs = ind1->free(j);
    ASSERT_TRUE(rs == SDK_RET_OK);
    rte_indexer::destroy(ind1);
}

//----------------------------------------------------------------------------
// Test 4:
//
// Summary:
// --------
//  - Tests the -ve case of indexer with size 0 and no allocations
//----------------------------------------------------------------------------
TEST_F(rte_indexer_test, test4) {
    uint32_t i;
    sdk_ret_t rs;

    // Instantiate the indexer
    rte_indexer *ind = rte_indexer::factory(0);

    // Allocate index
    rs  = ind->alloc(&i);
    ASSERT_TRUE(rs == SDK_RET_NO_RESOURCE);
    rte_indexer::destroy(ind);
}


//----------------------------------------------------------------------------
// Test 5:
//
// Summary:
// --------
//  - Tests all allowed allocs
//----------------------------------------------------------------------------
TEST_F(rte_indexer_test, test5) {
    sdk_ret_t rs;
    uint32_t id;

    // Instantiate the indexer
    rte_indexer *ind = rte_indexer::factory(10);

    // Loop to allocate max number of indices
    for (int i = 0; i < 10; i++) {
        rs  = ind->alloc(&id);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }
    rte_indexer::destroy(ind);
}

//----------------------------------------------------------------------------
// Test 6:
//
// Summary:
// --------
//  - Tests the limits of allocs for an indexer
//----------------------------------------------------------------------------
TEST_F(rte_indexer_test, test6) {
    sdk_ret_t rs;
    uint32_t id;

    // Instantiate the indexer
    rte_indexer *ind = rte_indexer::factory(10);

    // Loop to allocate max number of indices
    for (int i = 0; i < 10; i++) {
        rs  = ind->alloc(&id);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    // Extra allocation
    rs  = ind->alloc(&id);
    ASSERT_TRUE(rs == SDK_RET_NO_RESOURCE);
    rte_indexer::destroy(ind);
}

//----------------------------------------------------------------------------
// Test 7:
//
// Summary:
// --------
//  - Tests the -ve case of multiple free
//----------------------------------------------------------------------------
TEST_F(rte_indexer_test, test7) {
    uint32_t i;

    // Instantiate the indexer
    rte_indexer *ind = rte_indexer::factory(100);

    // Allocate index
    sdk_ret_t rs  = ind->alloc(&i);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Free index
    rs = ind->free(i);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Duplicate Free index
    rs = ind->free(i);
    ASSERT_TRUE(rs == SDK_RET_ENTRY_NOT_FOUND);
    rte_indexer::destroy(ind);
}

//----------------------------------------------------------------------------
// Test 8:
//
// Summary:
// --------
//  - Tests the -ve case of duplicate allocs with "with_id"
//----------------------------------------------------------------------------
TEST_F(rte_indexer_test, test8) {
    uint32_t i;

    // Instantiate the indexer
    rte_indexer *ind = rte_indexer::factory(100);

    // Allocate index
    sdk_ret_t rs  = ind->alloc(&i);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Duplicate allocate with same index
    rs  = ind->alloc(i);
    ASSERT_TRUE(rs == SDK_RET_ENTRY_EXISTS);

    // Allocate with new index
    rs  = ind->alloc(++i);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Duplicate allocate with same index
    rs  = ind->alloc(i);
    ASSERT_TRUE(rs == SDK_RET_ENTRY_EXISTS);

    // Free index
    rs = ind->free(i);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Free index
    rs = ind->free(--i);
    ASSERT_TRUE(rs == SDK_RET_OK);
    rte_indexer::destroy(ind);
}

//----------------------------------------------------------------------------
// Test 9:
//
// Summary:
// --------
//  - Tests the -ve case of Out of Bound (OOB) indices
//  - Tests the alloc and free of the maximum allowed index
//----------------------------------------------------------------------------
TEST_F(rte_indexer_test, test9) {
    uint32_t i;

    // Instantiate the indexer
    rte_indexer *ind = rte_indexer::factory(100);

    // Allocate index
    sdk_ret_t rs  = ind->alloc(&i);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Allocate index with_id with OOB index
    rs = ind->alloc(101);
    ASSERT_TRUE(rs == SDK_RET_OOB);

    // Free index with OOB index
    rs = ind->free(101);
    ASSERT_TRUE(rs == SDK_RET_OOB);

    // Allocate index with_id with OOB index
    rs = ind->alloc(100);
    ASSERT_TRUE(rs == SDK_RET_OOB);

    // Free index with OOB index
    rs = ind->free(100);
    ASSERT_TRUE(rs == SDK_RET_OOB);

    // Allocate the max index
    rs = ind->alloc(99);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Free the max index
    rs = ind->free(99);
    ASSERT_TRUE(rs == SDK_RET_OK);
    rte_indexer::destroy(ind);
}

//----------------------------------------------------------------------------
// Test 10:
//
// Summary:
// --------
//  - Testcase of multiple alloc and free
//----------------------------------------------------------------------------
TEST_F(rte_indexer_test, test10) {
    uint32_t i;

    // Instantiate the indexer
    rte_indexer *ind = rte_indexer::factory(100);

    // Allocate index
    sdk_ret_t rs  = ind->alloc(&i);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Free index
    rs = ind->free(i);
    ASSERT_TRUE(rs == SDK_RET_OK);

    // Allocate same index
    rs = ind->alloc(i);
    ASSERT_TRUE(rs == SDK_RET_OK);
    rte_indexer::destroy(ind);
}

//----------------------------------------------------------------------------
// Test 11:
//
// Summary:
// --------
//  - Testcase to verify allocating next available bit always
//----------------------------------------------------------------------------
TEST_F(rte_indexer_test, test11) {
    uint32_t i, j;
    sdk_ret_t rs;

    // Instantiate the indexer
    rte_indexer  *ind = rte_indexer::factory(4096);

    for (auto c = 0; c < 4000; c++) {
        // Allocate index
        rs  = ind->alloc(&i);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    for (auto c = 0; c < 1000; c++) {
        rs = ind->free(c);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    rs = ind->alloc(&j);
    ASSERT_TRUE(rs == SDK_RET_OK);
    ASSERT_TRUE(j == (i + 1));

    rte_indexer::destroy(ind);
}

//----------------------------------------------------------------------------
// Test 12:
//
// Summary:
// --------
//  - Testcase to verify wrap around of indexer
//----------------------------------------------------------------------------
TEST_F(rte_indexer_test, test12) {
    uint32_t i, j;
    sdk_ret_t rs;

    // Instantiate the indexer
    rte_indexer  *ind = rte_indexer::factory(4096);

    for (auto c = 0; c < 4096; c++) {
        // Allocate index
        rs  = ind->alloc(&i);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    for (auto c = 0; c < 1000; c++) {
        rs = ind->free(c);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    rs = ind->alloc(&j);
    ASSERT_TRUE(rs == SDK_RET_OK);
    ASSERT_TRUE(j == (i + 1) % 4096);

    rte_indexer::destroy(ind);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
