#include "nic/utils/slab/slab.hpp"
#include <gtest/gtest.h>

using hal::utils::slab;

class slab_test : public ::testing::Test {
protected:
    slab_test() {
    }

    virtual ~slab_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

// test that slab creation fails with invalid args to constructor
TEST_F(slab_test, create_fail) {
    slab *test_slab;

    // invalid args
    test_slab = slab::factory("test", hal::HAL_SLAB_RSVD, 0, 0);
    ASSERT_TRUE(test_slab == NULL);

    delete test_slab;
}

TEST_F(slab_test, create_ok) {
    slab *test_slab;

    test_slab = slab::factory("test", hal::HAL_SLAB_RSVD, 12, 2, false, true);
    ASSERT_TRUE(test_slab != NULL);
    delete test_slab;
}

TEST_F(slab_test, deplete_slab_blocks) {
    slab *test_slab;
    void *obj1, *obj2, *obj3;

    test_slab = slab::factory("test", hal::HAL_SLAB_RSVD, 12, 2, false, false);
    ASSERT_TRUE(test_slab != NULL);

    obj1 = test_slab->alloc();
    ASSERT_TRUE(obj1 != NULL);
    ASSERT_EQ(1U, test_slab->num_allocs());
    ASSERT_EQ(1U, test_slab->num_in_use());

    obj2 = test_slab->alloc();
    ASSERT_TRUE(obj2 != NULL);
    ASSERT_EQ(2U, test_slab->num_allocs());
    ASSERT_EQ(2U, test_slab->num_in_use());

    // make sure the next alloc fails
    obj3 = test_slab->alloc();
    ASSERT_TRUE(obj3 == NULL);
    ASSERT_EQ(2U, test_slab->num_allocs());
    ASSERT_EQ(2U, test_slab->num_in_use());
    ASSERT_EQ(1U, test_slab->num_alloc_fails());

    // free the objects in reverse order
    test_slab->free(obj2);
    test_slab->free(obj1);
    ASSERT_EQ(2U, test_slab->num_frees());
    ASSERT_EQ(0U, test_slab->num_in_use());

    // try to realloc and make sure they succeed
    obj1 = test_slab->alloc();
    ASSERT_TRUE(obj1 != NULL);
    obj2 = test_slab->alloc();
    ASSERT_TRUE(obj2 != NULL);
    ASSERT_EQ(4U, test_slab->num_allocs());
    ASSERT_EQ(2U, test_slab->num_in_use());
    obj3 = test_slab->alloc();
    ASSERT_TRUE(obj3 == NULL);
    ASSERT_EQ(2U, test_slab->num_alloc_fails());

    delete test_slab;
}

TEST_F(slab_test, grow_on_demand) {
    slab *test_slab;
    void *obj1, *obj2, *obj3;

    test_slab = slab::factory("test", hal::HAL_SLAB_RSVD, 12, 2, false, true);
    ASSERT_TRUE(test_slab != NULL);

    obj1 = test_slab->alloc();
    ASSERT_TRUE(obj1 != NULL);
    ASSERT_EQ(1, test_slab->num_allocs());
    ASSERT_EQ(1, test_slab->num_in_use());

    obj2 = test_slab->alloc();
    ASSERT_TRUE(obj2 != NULL);
    ASSERT_EQ(2, test_slab->num_allocs());
    ASSERT_EQ(2, test_slab->num_in_use());

    // make sure the next alloc fails
    obj3 = test_slab->alloc();
    ASSERT_TRUE(obj3 != NULL);

    test_slab->free(obj3);
    test_slab->free(obj1);
    test_slab->free(obj2);
    ASSERT_EQ(3, test_slab->num_frees());
    ASSERT_EQ(0, test_slab->num_in_use());

    delete test_slab;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
