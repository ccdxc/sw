#include "nic/utils/indexer/indexer.hpp"
#include "gtest/gtest.h"
#include <stdio.h>

using hal::utils::indexer;

namespace hal {
    namespace utils {
        hal::utils::mem_mgr     g_hal_mem_mgr;
    }
}

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



// ----------------------------------------------------------------------------
// Test 1:
//      - Create indexer
//      - Alloc index.
//      - Free index.
// ----------------------------------------------------------------------------
TEST_F(indexer_test, test1) {
    indexer *ind = indexer::factory(100);

    uint32_t i;
    indexer::status rs  = ind->alloc(&i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    rs = ind->free(i);
    ASSERT_TRUE(rs == indexer::SUCCESS);
}

// ----------------------------------------------------------------------------
// Test 2:
//      - Create indexer
//      - Alloc index.
//      - Alloc index.
//      - Free index.
//      - Free index.
// ----------------------------------------------------------------------------
TEST_F(indexer_test, test2) {
    indexer::status rs = indexer::SUCCESS;
    uint32_t i,j;
    indexer *ind1 = indexer::factory(100);

    rs  = ind1->alloc(&i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    rs  = ind1->alloc(&j);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    rs = ind1->free(i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    rs = ind1->free(j);
    ASSERT_TRUE(rs == indexer::SUCCESS);
}

// ----------------------------------------------------------------------------
// Test 3:
//      - Create indexer
//      - Alloc index.
//      - Free index.
//      - Alloc index.
//      - Free index.
// ----------------------------------------------------------------------------
TEST_F(indexer_test, test3) {
    indexer::status rs = indexer::SUCCESS;
    uint32_t i,j;
    indexer *ind1 = indexer::factory(100);

    rs  = ind1->alloc(&i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    rs = ind1->free(i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    rs  = ind1->alloc(&j);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    rs = ind1->free(j);
    ASSERT_TRUE(rs == indexer::SUCCESS);
}

// ----------------------------------------------------------------------------
// Test 4:
//      - Create indexer with size 0
//      - Alloc index. Returns failure
// ----------------------------------------------------------------------------
TEST_F(indexer_test, test4) {
    indexer *ind = indexer::factory(0);

    uint32_t i;
    indexer::status rs  = ind->alloc(&i);
    ASSERT_TRUE(rs == indexer::INVALID_INDEXER);
}


// ----------------------------------------------------------------------------
// Test 5:
//      - Create indexer with size 10
//      - 10 indices
// ----------------------------------------------------------------------------
TEST_F(indexer_test, test5) {
    indexer *ind = indexer::factory(10);
    indexer::status rs;

    uint32_t id;

    for (int i = 0; i < 10; i++) {
        rs  = ind->alloc(&id);
        ASSERT_TRUE(rs == indexer::SUCCESS);
    }
}

// ----------------------------------------------------------------------------
// Test 6:
//      - Create indexer with size 10
//      - 11 indices
// ----------------------------------------------------------------------------
TEST_F(indexer_test, test6) {
    indexer *ind = indexer::factory(10);
    indexer::status rs;

    uint32_t id;

    for (int i = 0; i < 10; i++) {
        rs  = ind->alloc(&id);
        ASSERT_TRUE(rs == indexer::SUCCESS);
    }
    rs  = ind->alloc(&id);
    ASSERT_TRUE(rs == indexer::INDEXER_FULL);
}

// ----------------------------------------------------------------------------
// Test 7:
//      - Create indexer
//      - Alloc index.
//      - Free index.
//      - Free index.
// ----------------------------------------------------------------------------
TEST_F(indexer_test, test7) {
    indexer *ind = indexer::factory(100);

    uint32_t i;
    indexer::status rs  = ind->alloc(&i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    rs = ind->free(i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    rs = ind->free(i);
    ASSERT_TRUE(rs == indexer::DUPLICATE_FREE);
}

// ----------------------------------------------------------------------------
// Test 8:
//      - Create indexer
//      - Alloc index.
//      - Alloc index with id
// ----------------------------------------------------------------------------
TEST_F(indexer_test, test8) {
    indexer *ind = indexer::factory(100);

    uint32_t i;
    indexer::status rs  = ind->alloc(&i);
    ASSERT_TRUE(rs == indexer::SUCCESS);


    rs  = ind->alloc_withid(i);
    ASSERT_TRUE(rs == indexer::DUPLICATE_ALLOC);

    rs  = ind->alloc_withid(++i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    rs  = ind->alloc_withid(i);
    ASSERT_TRUE(rs == indexer::DUPLICATE_ALLOC);

    rs = ind->free(i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    rs = ind->free(--i);
    ASSERT_TRUE(rs == indexer::SUCCESS);
}

// ----------------------------------------------------------------------------
// Test 9:
//      - Create indexer
//      - Alloc index.
//      - -ve cases
// ----------------------------------------------------------------------------
TEST_F(indexer_test, test9) {
    indexer *ind = indexer::factory(100);

    uint32_t i;
    indexer::status rs  = ind->alloc(&i);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    rs = ind->alloc_withid(101);
    ASSERT_TRUE(rs == indexer::INDEX_OOB);

    rs = ind->free(101);
    ASSERT_TRUE(rs == indexer::INDEX_OOB);

    rs = ind->alloc_withid(100);
    ASSERT_TRUE(rs == indexer::INDEX_OOB);

    rs = ind->free(100);
    ASSERT_TRUE(rs == indexer::INDEX_OOB);

    rs = ind->alloc_withid(99);
    ASSERT_TRUE(rs == indexer::SUCCESS);

    rs = ind->free(99);
    ASSERT_TRUE(rs == indexer::SUCCESS);
}

// ----------------------------------------------------------------------------
// Test 10:
//      - Create indexer
//      - Alloc index - bottom up
//      - -ve cases
// ----------------------------------------------------------------------------
TEST_F(indexer_test, test10) {
    indexer::status rs;
    uint32_t i;

    indexer *ind1 = indexer::factory(128);
    indexer *ind2 = indexer::factory(100);

    rs  = ind1->alloc(&i, FALSE);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 127);

    rs = ind1->alloc_withid(130);
    EXPECT_EQ(rs, indexer::INDEX_OOB);

    rs  = ind1->alloc(&i, FALSE);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 126);

    rs = ind1->free(127);
    EXPECT_EQ(rs, indexer::SUCCESS);

    rs  = ind1->alloc(&i, FALSE);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 125);

    rs  = ind1->alloc(&i);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 0);
    
    rs = ind1->free(125);
    EXPECT_EQ(rs, indexer::SUCCESS);

    rs = ind1->free(127);
    EXPECT_EQ(rs, indexer::DUPLICATE_FREE);

    rs  = ind1->alloc(&i, FALSE);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 125);

    // Second indexer
    rs  = ind2->alloc(&i, FALSE);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 99);

    rs = ind2->alloc_withid(130);
    EXPECT_EQ(rs, indexer::INDEX_OOB);

    rs = ind2->alloc_withid(128);
    EXPECT_EQ(rs, indexer::INDEX_OOB);

    rs  = ind2->alloc(&i, FALSE);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 98);

    rs  = ind2->alloc(&i, FALSE);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 97);

    rs  = ind2->alloc(&i);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 0);

    rs = ind2->free(99);
    EXPECT_EQ(rs, indexer::SUCCESS);

    rs  = ind2->alloc(&i, FALSE);
    EXPECT_EQ(rs, indexer::SUCCESS);
    EXPECT_EQ(i, (uint32_t) 96);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
