// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <stdio.h>
#include "gtest/gtest.h"
#include "sdk/shmmgr.hpp"

using namespace sdk::lib;

class shmmgr_test : public ::testing::Test {
protected:
    shmmgr_test() {
    }

    virtual ~shmmgr_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

TEST_F(shmmgr_test, create_ok) {
    shmmgr *test_shmmgr;
    std::size_t    one_gig = 1024 * 1024 * 1024;

    test_shmmgr = shmmgr::factory("test-seg1", one_gig,
                                  OPEN_OR_CREATE, (void *)0xB0000000L);
    ASSERT_TRUE(test_shmmgr != NULL);
    ASSERT_TRUE(test_shmmgr->size() == one_gig);

    shmmgr::destroy(test_shmmgr);
}

TEST_F(shmmgr_test, alloc_dealloc_ok) {
    shmmgr         *test_shmmgr;
    std::size_t    one_mb = 1024 * 1024;
    void           *ptr1, *ptr2;
    uint32_t       free_size;

    try {
    test_shmmgr = shmmgr::factory("test-seg2", one_mb,
                             OPEN_OR_CREATE, (void *)0xA0000000L);
    } catch (boost::interprocess::bad_alloc &ex) {
        std::cerr << ex.what() << '\n';
    } catch (std::exception ex) {
        std::cerr << ex.what() << '\n';
    }

    ASSERT_TRUE(test_shmmgr != NULL);
    ASSERT_TRUE(test_shmmgr->size() == one_mb);
    free_size = test_shmmgr->free_size();
    ptr1 = test_shmmgr->allocate(512, 4);
    ASSERT_TRUE(ptr1 != NULL);
    // all max. possible out of rest of the memory available
    ptr2 = test_shmmgr->allocate(test_shmmgr->free_size() - 24, 0);
    ASSERT_TRUE(ptr2 != NULL);

    test_shmmgr->deallocate(ptr1);
    test_shmmgr->deallocate(ptr2);
    ASSERT_TRUE(test_shmmgr->free_size() == free_size);
    shmmgr::destroy(test_shmmgr);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

