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

TEST_F(rte_indexer_test, alloc_test1M_thread_unsafe) {
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

TEST_F(rte_indexer_test, alloc_test1M_thread_safe) {
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

    ind = rte_indexer::factory(4096);
    rs = ind->alloc_block(&i, 4096);
    ASSERT_TRUE(rs == SDK_RET_OK);
    rs = ind->alloc(&i);
    ASSERT_TRUE(rs == SDK_RET_NO_RESOURCE);
    rs = ind->free(0, 500);
    ASSERT_TRUE(rs == SDK_RET_OK);
    rs = ind->free(3595, 500);
    ASSERT_TRUE(rs == SDK_RET_OK);
    j = ind->usage();
    ASSERT_TRUE(j == 3096);
    rs = ind->alloc_block(&i, 1000);
    ASSERT_TRUE(rs == SDK_RET_NO_RESOURCE);
    rs = ind->alloc_block(&i, 500);
    ASSERT_TRUE(rs == SDK_RET_OK);
    rte_indexer::destroy(ind);
}

//----------------------------------------------------------------------------
// Test 13:
//
// Summary:
// --------
//  - Testcase of combinations of block free
//----------------------------------------------------------------------------
TEST_F(rte_indexer_test, test13) {
    uint32_t i;
    sdk_ret_t rs;

    // Instantiate the indexer
    rte_indexer *ind = rte_indexer::factory(2048);

    for (auto c = 0; c < 1024; c++) {
        rs = ind->alloc(&i);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    rs = ind->free(0, 1024);
    ASSERT_TRUE(rs == SDK_RET_OK);

    for (auto c = 0; c < 2048; c++) {
        rs = ind->alloc(&i);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    rs = ind->free(512, 1536);
    ASSERT_TRUE(rs == SDK_RET_OK);

    rs = ind->free(0, 512);
    ASSERT_TRUE(rs == SDK_RET_OK);
    rte_indexer::destroy(ind);

    ind = rte_indexer::factory(100000);
    for (auto c = 0; c < 100000; c++) {
        rs = ind->alloc(&i);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    i = 0;
    for (auto c = 0; c < 100000; c = c + 5000) {
        rs = ind->free(i, 5000);
        i = i + 5000;
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    rs = ind->alloc_block(&i, 100000);
    ASSERT_TRUE(rs == SDK_RET_OK);
    rs = ind->free(i, 100000);
    ASSERT_TRUE(rs == SDK_RET_OK);

    rte_indexer::destroy(ind);
}

//----------------------------------------------------------------------------
// Test 14:
//
// Summary:
// --------
//  - Testcase of combinations of block allocation using alloc_block
//----------------------------------------------------------------------------
TEST_F(rte_indexer_test, test14) {
    uint32_t i, j;
    sdk_ret_t rs;

    // Instantiate the indexer
    rte_indexer *ind = rte_indexer::factory(2048);

    for (auto c = 0; c < 1024; c++) {
        rs = ind->alloc(&i);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }
    j = i;
    rs = ind->alloc_block(&i, 1024);
    ASSERT_TRUE(rs == SDK_RET_OK);
    ASSERT_TRUE(i == j + 1);

    rs = ind->free(0, 2048);
    ASSERT_TRUE(rs == SDK_RET_OK);

    rs = ind->alloc_block(&i, 2049);
    ASSERT_TRUE(rs == SDK_RET_NO_RESOURCE);

    rs = ind->alloc_block(&i, 300);
    ASSERT_TRUE(i == 0);
    j = i + 300;
    for (auto c = 300; c < 900; c++) {
        rs = ind->alloc(&i);
        ASSERT_TRUE(i == j);
        ASSERT_TRUE(rs == SDK_RET_OK);
        j++;
    }
    j = i;

    rs = ind->alloc_block(&i, 1000);
    ASSERT_TRUE(rs == SDK_RET_OK);
    ASSERT_TRUE(i == j + 1);
    j = i + 1000;

    rs = ind->alloc_block(&i, 200);
    ASSERT_TRUE(rs == SDK_RET_NO_RESOURCE);

    rs = ind->alloc_block(&i, 148);
    ASSERT_TRUE(rs == SDK_RET_OK);
    ASSERT_TRUE(i == j);

    rte_indexer::destroy(ind);
}

//----------------------------------------------------------------------------
// Test 15:
//
// Summary:
// --------
//  - Testcase of max block allocation using alloc_block
//----------------------------------------------------------------------------
TEST_F(rte_indexer_test, alloc_bloc_1M_threadsafe) {
    uint32_t i;
    sdk_ret_t rs;

    // Instantiate the indexer
    rte_indexer *ind = rte_indexer::factory(1024*1024);

    for (auto c = 0; c < 1024*1024; c++) {
        rs = ind->alloc_block(&i);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }
    rs = ind->alloc_block(&i);
    ASSERT_TRUE(rs == SDK_RET_NO_RESOURCE);
    rte_indexer::destroy(ind);
}

//----------------------------------------------------------------------------
// Test 15:
//
// Summary:
// --------
//  - Testcase of block allocation/free using alloc_block and free
//----------------------------------------------------------------------------
TEST_F(rte_indexer_test, test15) {
    uint32_t i, size, j;
    sdk_ret_t rs;

    // Instantiate the indexer
    rte_indexer *ind = rte_indexer::factory(1024*1024);
    rs = ind->alloc_block(&i, 1024*1024);
    ASSERT_TRUE(rs == SDK_RET_OK);
    rs = ind->free(i, 1024*1024);
    ASSERT_TRUE(rs == SDK_RET_OK);
    rte_indexer::destroy(ind);

    ind = rte_indexer::factory(64);
    rs = ind->alloc_block(&i, 2);
    ASSERT_TRUE(rs == SDK_RET_OK);
    ASSERT_TRUE(i == 0);
    rs = ind->alloc(1);
    ASSERT_TRUE(rs == SDK_RET_ENTRY_EXISTS);

    for (auto c = 2; c < 64; c = c + 2) {
        rs = ind->alloc(c);
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    rs = ind->alloc_block(&i, 2);
    ASSERT_TRUE(rs == SDK_RET_NO_RESOURCE);

    j = 3;
    size = 64 - (ind->usage());
    for (auto c = 0; c < size; c++) {
        rs = ind->alloc_block(&i);
        ASSERT_TRUE(i == j);
        j = j + 2;
        ASSERT_TRUE(rs == SDK_RET_OK);
    }

    rte_indexer::destroy(ind);
}

//----------------------------------------------------------------------------
// Test 16:
//
// Summary:
// --------
//  - Testcase of combinations of alloc and alloc_block
//----------------------------------------------------------------------------
TEST_F(rte_indexer_test, test16) {
    uint32_t i;
    sdk_ret_t rs;

    // Instantiate the indexer
    rte_indexer *ind = rte_indexer::factory(1000);

    rs = ind->alloc(&i);
    ASSERT_TRUE(i == 0);
    ASSERT_TRUE(rs == SDK_RET_OK);

    rs = ind->alloc_block(&i, 998);
    ASSERT_TRUE(rs == SDK_RET_OK);
    ASSERT_TRUE(i == 1);

    rs = ind->alloc(&i);
    ASSERT_TRUE(i == 999);
    ASSERT_TRUE(rs == SDK_RET_OK);

    for (auto c = 0; c < 1000; c++) {
        rs = ind->alloc(c);
        ASSERT_TRUE(rs == SDK_RET_ENTRY_EXISTS);
    }
    rs = ind->alloc(&i);
    ASSERT_TRUE(rs == SDK_RET_NO_RESOURCE);

    rs = ind->free(900, 10);
    ASSERT_TRUE(rs == SDK_RET_OK);

    for (auto c = 900; c < 910; c++) {
        i = ind->is_index_allocated(c);
        ASSERT_TRUE(i == 0);
    }

    rs = ind->alloc(&i);
    ASSERT_TRUE(rs == SDK_RET_OK);
    ASSERT_TRUE(i == 900);

    rs = ind->free(3, 10);
    ASSERT_TRUE(rs == SDK_RET_OK);

    for (auto c = 901; c < 910; c++) {
        rs = ind->alloc(&i);
        ASSERT_TRUE(rs == SDK_RET_OK);
        ASSERT_TRUE(i == c);
    }

    for (auto c = 3; c < 13; c++) {
        rs = ind->alloc(&i);
        ASSERT_TRUE(rs == SDK_RET_OK);
        ASSERT_TRUE(i == c);
    }

    rte_indexer::destroy(ind);
}

//----------------------------------------------------------------------------
// Test 17:
//
// Summary:
// --------
//  - Testcase of combinations of alloc and alloc_block
//----------------------------------------------------------------------------
TEST_F(rte_indexer_test, test17) {
    uint32_t i, j, usage, size;
    sdk_ret_t rs;

    // Instantiate the indexer
    size = 1048560;
    rte_indexer *ind = rte_indexer::factory(size);

    usage = 0;
    while (usage < size) {
        rs = ind->alloc_block(&i, 100);
        ASSERT_TRUE(rs == SDK_RET_OK);
        rs = ind->alloc(&j);
        ASSERT_TRUE(rs == SDK_RET_OK);
        ASSERT_TRUE(j == (i + 100));
        rs = ind->alloc(j + 1);
        ASSERT_TRUE(rs == SDK_RET_OK);
        usage = ind->usage();
    }

    for (auto c = 0; c < size; c++) {
        rs = ind->alloc(c);
        ASSERT_TRUE(rs == SDK_RET_ENTRY_EXISTS);
    }

    usage = size;
    i = 0;
    while (usage > 0) {
        rs = ind->free(i, 100);
        ASSERT_TRUE(rs == SDK_RET_OK);
        j = i + 100;
        rs = ind->free(j);
        ASSERT_TRUE(rs == SDK_RET_OK);
        rs = ind->free(j + 1);
        ASSERT_TRUE(rs == SDK_RET_OK);
        usage = ind->usage();
        i = j + 2;
    }

    rte_indexer::destroy(ind);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
