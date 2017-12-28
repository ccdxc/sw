//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// Test Cases to verify Indexer library
//------------------------------------------------------------------------------

#include <stdio.h>
#include "gtest/gtest.h"
#include "sdk/indexer.hpp"

using sdk::lib::indexer;

//------------------------------------------------------------------------------
// Indexer Test Class 
//------------------------------------------------------------------------------
class indexer_test : public ::testing::Test {
protected:
  indexer_test() {
  }

  virtual ~indexer_test() {
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
TEST_F(indexer_test, test1) {
    uint32_t i;

    // Instantiate the indexer
    indexer  *ind = indexer::factory(100);

    // Allocate index
    indexer::status rs  = ind->alloc(&i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    // Free index
    rs = ind->free(i);
    ASSERT_TRUE(rs == indexer::SUCCESS);
}

//----------------------------------------------------------------------------
// Test 2:
//
// Summary:
// --------
//  - Tests the case of multiple allocations and frees
//----------------------------------------------------------------------------
TEST_F(indexer_test, test2) {
    indexer::status rs = indexer::SUCCESS;
    uint32_t i,j;

    // Instantiate the indexer
    indexer *ind1 = indexer::factory(100);

    // Allocate index
    rs  = ind1->alloc(&i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    // Allocate index
    rs  = ind1->alloc(&j);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    // Free index
    rs = ind1->free(i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    // Free index
    rs = ind1->free(j);
    ASSERT_TRUE(rs == indexer::SUCCESS);
}

//----------------------------------------------------------------------------
// Test 3:
//
// Summary:
// --------
//  - Tests the interleaved multiple allocations and frees
//----------------------------------------------------------------------------
TEST_F(indexer_test, test3) {
    indexer::status rs = indexer::SUCCESS;
    uint32_t i,j;

    // Instantiate the indexer
    indexer *ind1 = indexer::factory(100);

    // Allocate index
    rs  = ind1->alloc(&i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    // Free index
    rs = ind1->free(i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    // Allocate index
    rs  = ind1->alloc(&j);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    // Free index
    rs = ind1->free(j);
    ASSERT_TRUE(rs == indexer::SUCCESS);
}

//----------------------------------------------------------------------------
// Test 4:
//
// Summary:
// --------
//  - Tests the -ve case of indexer with size 0 and no allocations
//----------------------------------------------------------------------------
TEST_F(indexer_test, test4) {
    uint32_t i;
    indexer::status rs;

    // Instantiate the indexer
    indexer *ind = indexer::factory(0);

    // Allocate index
    rs  = ind->alloc(&i);
    ASSERT_TRUE(rs == indexer::INVALID_INDEXER);
}


//----------------------------------------------------------------------------
// Test 5:
//
// Summary:
// --------
//  - Tests all allowed allocs
//----------------------------------------------------------------------------
TEST_F(indexer_test, test5) {
    indexer::status rs;
    uint32_t id;

    // Instantiate the indexer
    indexer *ind = indexer::factory(10);

    // Loop to allocate max number of indices
    for (int i = 0; i < 10; i++) {
        rs  = ind->alloc(&id);
        ASSERT_TRUE(rs == indexer::SUCCESS);
    }
}

//----------------------------------------------------------------------------
// Test 6:
//
// Summary:
// --------
//  - Tests the limits of allocs for an indexer
//----------------------------------------------------------------------------
TEST_F(indexer_test, test6) {
    indexer::status rs;
    uint32_t id;

    // Instantiate the indexer
    indexer *ind = indexer::factory(10);

    // Loop to allocate max number of indices
    for (int i = 0; i < 10; i++) {
        rs  = ind->alloc(&id);
        ASSERT_TRUE(rs == indexer::SUCCESS);
    }

    // Extra allocation
    rs  = ind->alloc(&id);
    ASSERT_TRUE(rs == indexer::INDEXER_FULL);
}

//----------------------------------------------------------------------------
// Test 7:
//
// Summary:
// --------
//  - Tests the -ve case of multiple free
//----------------------------------------------------------------------------
TEST_F(indexer_test, test7) {
    uint32_t i;

    // Instantiate the indexer
    indexer *ind = indexer::factory(100);

    // Allocate index
    indexer::status rs  = ind->alloc(&i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    // Free index
    rs = ind->free(i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    // Duplicate Free index
    rs = ind->free(i);
    ASSERT_TRUE(rs == indexer::DUPLICATE_FREE);
}

//----------------------------------------------------------------------------
// Test 8:
//
// Summary:
// --------
//  - Tests the -ve case of duplicate allocs with "with_id"
//----------------------------------------------------------------------------
TEST_F(indexer_test, test8) {
    uint32_t i;

    // Instantiate the indexer
    indexer *ind = indexer::factory(100);

    // Allocate index
    indexer::status rs  = ind->alloc(&i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    // Duplicate allocate with same index
    rs  = ind->alloc_withid(i);
    ASSERT_TRUE(rs == indexer::DUPLICATE_ALLOC);

    // Allocate with new index
    rs  = ind->alloc_withid(++i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    // Duplicate allocate with same index
    rs  = ind->alloc_withid(i);
    ASSERT_TRUE(rs == indexer::DUPLICATE_ALLOC);

    // Free index
    rs = ind->free(i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    // Free index
    rs = ind->free(--i);
    ASSERT_TRUE(rs == indexer::SUCCESS);
}

//----------------------------------------------------------------------------
// Test 9:
//
// Summary:
// --------
//  - Tests the -ve case of Out of Bound (OOB) indices
//  - Tests the alloc and free of the maximum allowed index
//----------------------------------------------------------------------------
TEST_F(indexer_test, test9) {
    uint32_t i;

    // Instantiate the indexer
    indexer *ind = indexer::factory(100);

    // Allocate index
    indexer::status rs  = ind->alloc(&i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    // Allocate index with_id with OOB index
    rs = ind->alloc_withid(101);
    ASSERT_TRUE(rs == indexer::INDEX_OOB);

    // Free index with OOB index
    rs = ind->free(101);
    ASSERT_TRUE(rs == indexer::INDEX_OOB);

    // Allocate index with_id with OOB index
    rs = ind->alloc_withid(100);
    ASSERT_TRUE(rs == indexer::INDEX_OOB);

    // Free index with OOB index
    rs = ind->free(100);
    ASSERT_TRUE(rs == indexer::INDEX_OOB);

    // Allocate the max index
    rs = ind->alloc_withid(99);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    // Free the max index
    rs = ind->free(99);
    ASSERT_TRUE(rs == indexer::SUCCESS);
}

//----------------------------------------------------------------------------
// Test 10:
//
// Summary:
// --------
//  - Tests multiple indexers with bottom up (from end) feature
//----------------------------------------------------------------------------
TEST_F(indexer_test, test10) {
    indexer::status rs;
    uint32_t i;

    // Instantiate the indexers
    indexer *ind1 = indexer::factory(128);
    indexer *ind2 = indexer::factory(100);

    // Allocate indexer from end
    rs  = ind1->alloc(&i, FALSE);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 127);

    // Allocate indexer to verify OOB
    rs = ind1->alloc_withid(130);
    EXPECT_EQ(rs, indexer::INDEX_OOB);

    // Allocate indexer from end
    rs  = ind1->alloc(&i, FALSE);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 126);

    // Free index
    rs = ind1->free(127);
    EXPECT_EQ(rs, indexer::SUCCESS);

    // Allocate index from end
    rs  = ind1->alloc(&i, FALSE);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 125);

    // Allocate index from start
    rs  = ind1->alloc(&i);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 0);
    
    // Free index
    rs = ind1->free(125);
    EXPECT_EQ(rs, indexer::SUCCESS);

    // Free index
    rs = ind1->free(127);
    EXPECT_EQ(rs, indexer::DUPLICATE_FREE);

    // Allocate index from end
    rs  = ind1->alloc(&i, FALSE);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 125);

    // Second indexer
    rs  = ind2->alloc(&i, FALSE);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 99);

    // Allocate index with_id to test OOB
    rs = ind2->alloc_withid(130);
    EXPECT_EQ(rs, indexer::INDEX_OOB);

    // Allocate index with_id to test OOB
    rs = ind2->alloc_withid(128);
    EXPECT_EQ(rs, indexer::INDEX_OOB);

    // Allocate index from end
    rs  = ind2->alloc(&i, FALSE);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 98);

    // Allocate index from end
    rs  = ind2->alloc(&i, FALSE);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 97);

    // Allocate index from start
    rs  = ind2->alloc(&i);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 0);

    // Free index
    rs = ind2->free(99);
    EXPECT_EQ(rs, indexer::SUCCESS);

    // Allocate index from end
    rs  = ind2->alloc(&i, FALSE);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 96);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
