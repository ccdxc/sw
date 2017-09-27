#include "nic/utils/shm/shmseg.hpp"
#include "gtest/gtest.h"

using namespace utils::shm;

class shmseg_test : public ::testing::Test {
protected:
    shmseg_test() {
    }

    virtual ~shmseg_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

TEST_F(shmseg_test, create_ok) {
    shmseg *test_shmseg;
    std::size_t    one_gig = 1024 * 1024 * 1024;

    test_shmseg = new shmseg("test-seg1", one_gig,
                             OPEN_OR_CREATE, (void *)0xB0000000L);
    ASSERT_TRUE(test_shmseg != NULL);
    ASSERT_TRUE(test_shmseg->size() == one_gig);

    delete test_shmseg;
}

TEST_F(shmseg_test, alloc_dealloc_ok) {
    shmseg         *test_shmseg;
    std::size_t    one_mb = 1024 * 1024;
    void           *ptr1, *ptr2;
    uint32_t       free_size;

    try {
    test_shmseg = new shmseg("test-seg2", one_mb,
                             OPEN_OR_CREATE, (void *)0xA0000000L);
    } catch (boost::interprocess::bad_alloc &ex) {
        std::cerr << ex.what() << '\n';
    } catch (std::exception ex) {
        std::cerr << ex.what() << '\n';
    }

    ASSERT_TRUE(test_shmseg != NULL);
    ASSERT_TRUE(test_shmseg->size() == one_mb);
    free_size = test_shmseg->free_size();
    ptr1 = test_shmseg->allocate(512, 4);
    ASSERT_TRUE(ptr1 != NULL);
    // all max. possible out of rest of the memory available
    ptr2 = test_shmseg->allocate(test_shmseg->free_size() - 24, 0);
    ASSERT_TRUE(ptr2 != NULL);

    test_shmseg->deallocate(ptr1);
    test_shmseg->deallocate(ptr2);
    ASSERT_TRUE(test_shmseg->free_size() == free_size);
    delete test_shmseg;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

