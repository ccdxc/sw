#include <bitmap.hpp>
#include <gtest/gtest.h>
#include <stdio.h>

using hal::utils::bitmap;

class bitmap_test : public ::testing::Test {
protected:
    bitmap_test() {
    }

    virtual ~bitmap_test() {
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
//      - Create bitmap
//      - Set bit.
//      - Clear bit.
// ----------------------------------------------------------------------------
TEST_F(bitmap_test, test1) {
    bitmap *bmp = bitmap::factory(100);

    uint32_t i = 10;
    hal_ret_t rs  = bmp->set(i);
    ASSERT_EQ(rs, HAL_RET_OK);

    rs = bmp->clear(i);
    ASSERT_EQ(rs, HAL_RET_OK);

    delete bmp;
}

// ----------------------------------------------------------------------------
// Test 2:
//      - Create bitmap
//      - Set bit.
//      - Set bit.
//      - Clear bit.
//      - Clear bit.
// ----------------------------------------------------------------------------
TEST_F(bitmap_test, test2) {
    hal_ret_t rs = HAL_RET_OK;
    uint32_t i,j;
    bitmap *bmp1 = bitmap::factory(100);

    i = 10;
    j = 20;

    rs  = bmp1->set(i);
    ASSERT_EQ(rs, HAL_RET_OK);

    rs  = bmp1->set(j);
    ASSERT_EQ(rs, HAL_RET_OK);

    rs = bmp1->clear(i);
    ASSERT_EQ(rs, HAL_RET_OK);

    rs = bmp1->clear(j);
    ASSERT_EQ(rs, HAL_RET_OK);

    delete bmp1;
}

// ----------------------------------------------------------------------------
// Test 3:
//      - Create bitmap
//      - Set bit.
//      - Clear bit.
//      - Set bit.
//      - Clear bit.
// ----------------------------------------------------------------------------
TEST_F(bitmap_test, test3) {
    hal_ret_t rs = HAL_RET_OK;
    uint32_t i,j;
    bitmap *bmp1 = bitmap::factory(100);

    i = 1;
    j = 2;
    rs  = bmp1->set(i);
    ASSERT_EQ(rs, HAL_RET_OK);

    rs = bmp1->clear(i);
    ASSERT_EQ(rs, HAL_RET_OK);

    rs  = bmp1->set(j);
    ASSERT_EQ(rs, HAL_RET_OK);

    rs = bmp1->clear(j);
    ASSERT_EQ(rs, HAL_RET_OK);

    delete bmp1;
}

// ----------------------------------------------------------------------------
// Test 4:
//      - Create bitmap with size 0
//      - Set bit. Returns failure
// ----------------------------------------------------------------------------
TEST_F(bitmap_test, test4) {
    bitmap *bmp = bitmap::factory(0);

    hal_ret_t rs  = bmp->set(10);
    ASSERT_EQ(rs, HAL_RET_NO_RESOURCE);

    delete bmp;
}


// ----------------------------------------------------------------------------
// Test 5:
//      - Create bitmap with size 10
//      - 10 sets
//      - Negative test for 11th set
// ----------------------------------------------------------------------------
TEST_F(bitmap_test, test5) {
    bitmap *bmp = bitmap::factory(10);
    hal_ret_t rs;

    for (int i = 0; i < 10; i++) {
        rs  = bmp->set(i);
        ASSERT_EQ(rs, HAL_RET_OK);
    }

    delete bmp;
}

// ----------------------------------------------------------------------------
// Test 6:
//      - Create bitmap with size 10
//      - 11 bmpices
// ----------------------------------------------------------------------------
TEST_F(bitmap_test, test6) {
    bitmap *bmp = bitmap::factory(10);
    hal_ret_t rs;
    uint32_t i;

    for (i = 0; i < 10; i++) {
        rs  = bmp->set(i);
        ASSERT_EQ(rs, HAL_RET_OK);
    }
    rs  = bmp->set(i);
    ASSERT_EQ(rs, HAL_RET_NO_RESOURCE);

    delete bmp;
}

// ----------------------------------------------------------------------------
// Test 7:
//      - Create bitmap
//      - Set bit.
//      - -ve cases
// ----------------------------------------------------------------------------
TEST_F(bitmap_test, test7) {
    bitmap *bmp = bitmap::factory(100);

    uint32_t p;

    hal_ret_t rs  = bmp->set(10);
    ASSERT_EQ(rs, HAL_RET_OK);

    rs = bmp->first_set(&p);
    ASSERT_EQ(rs, HAL_RET_OK);
    ASSERT_EQ(p, (unsigned)10);

    rs = bmp->next_set(9, &p);
    ASSERT_EQ(rs, HAL_RET_OK);
    ASSERT_EQ(p, (unsigned)10);

    rs = bmp->prev_set(11, &p);
    ASSERT_EQ(rs, HAL_RET_OK);
    ASSERT_EQ(p, (unsigned)10);

    rs = bmp->first_free(&p);
    ASSERT_EQ(rs, HAL_RET_OK);
    ASSERT_EQ(p, (unsigned)0);

    rs = bmp->next_free(9, &p);
    ASSERT_EQ(rs, HAL_RET_OK);
    ASSERT_EQ(p, (unsigned)11);

    rs = bmp->prev_free(11, &p);
    ASSERT_EQ(rs, HAL_RET_OK);
    ASSERT_EQ(p, (unsigned)9);

    rs = bmp->clear(101);
    ASSERT_EQ(rs, HAL_RET_NO_RESOURCE);

    rs = bmp->set(100);
    ASSERT_EQ(rs, HAL_RET_NO_RESOURCE);

    rs = bmp->clear(99);
    ASSERT_EQ(rs, HAL_RET_OK);

    delete bmp;
}

static void 
test_one(int seed_, int no_bits) 
{
    hal_ret_t rs;
    int i, j;
    uint32_t p;
    bool is_set;

    int32_t first_set;
    int32_t first_free;
    bool set[no_bits];
    int32_t next_set[no_bits];
    int32_t next_free[no_bits];
    int32_t prev_set[no_bits];
    int32_t prev_free[no_bits];

    bitmap *bmp = bitmap::factory(no_bits);

    std::cout << "Random seed used " << seed_ << std::endl;
    srand(seed_);

    first_set = -1;
    first_free = -1;
    for (i = 0; i < no_bits; i++) {
        set[i] = false;
        next_set[i] = -1;
        next_free[i] = -1;
        prev_set[i] = -1;
        prev_free[i] = -1;
    }

    for (i = 0; i < no_bits; i++) {
        is_set = bmp->is_set(i);
        ASSERT_EQ(is_set, set[i]);
    }

    for (i = 0; i < no_bits; i++) {
        if (rand()&0x1) {
            bmp->set(i);
            set[i] = true;

            if (first_set == -1) {
                first_set = i;
            }

            for (j = i-1; (j >= 0) && (set[j] == false); j--) {
                next_set[j] = i;
            }
            next_set[j] = i;

            for (j = i + 1; j < no_bits; j++) {
                prev_set[j] = i;
            }
        } else {
            if (first_free == -1) {
                first_free = i;
            }

            for (j = i-1; (j >= 0) && (set[j] == true); j--) {
                next_free[j] = i;
            }
            next_free[j] = i;
            for (j = i + 1; j < no_bits; j++) {
                prev_free[j] = i;
            }
        }
    }

    for (i = 0; i < no_bits; i++) {
        is_set = bmp->is_set(i);
        ASSERT_EQ(is_set, set[i]);
    }

    rs = bmp->first_set(&p);
    ASSERT_EQ(rs, HAL_RET_OK);
    ASSERT_EQ(p, (unsigned)first_set);

    rs = bmp->first_free(&p);
    ASSERT_EQ(rs, HAL_RET_OK);
    ASSERT_EQ(p, (unsigned)first_free);

    for (i = 0; i < no_bits; i++) {
        rs = bmp->next_set(i, &p);
        if (next_set[i] != -1) {
            ASSERT_EQ(rs, HAL_RET_OK);
            ASSERT_EQ(p, (unsigned)next_set[i]);
        } else {
            ASSERT_EQ(rs, HAL_RET_NO_RESOURCE);
        }

        rs = bmp->prev_set(i, &p);
        if (prev_set[i] != -1) {
            ASSERT_EQ(rs, HAL_RET_OK);
            ASSERT_EQ(p, (unsigned)prev_set[i]);
        } else {
            ASSERT_EQ(rs, HAL_RET_NO_RESOURCE);
        }

        rs = bmp->next_free(i, &p);
        if (next_free[i] != -1) {
            ASSERT_EQ(rs, HAL_RET_OK);
            ASSERT_EQ(p, (unsigned)next_free[i]);
        } else {
            ASSERT_EQ(rs, HAL_RET_NO_RESOURCE);
        }

        rs = bmp->prev_free(i, &p);
        if (prev_free[i] != -1) {
            ASSERT_EQ(rs, HAL_RET_OK);
            ASSERT_EQ(p, (unsigned)prev_free[i]);
        } else {
            ASSERT_EQ(rs, HAL_RET_NO_RESOURCE);
        }
    }
    std::cout << "Random seed used " << seed_ << std::endl;
    delete bmp;
}

// ----------------------------------------------------------------------------
// Test 8:
//      - Create bitmap
//      - Test the first/next/prev functions
// ----------------------------------------------------------------------------
TEST_F(bitmap_test, test8) 
{
    int seed_;

    seed_ = rand();
    ASSERT_NO_FATAL_FAILURE(test_one(seed_, 1025));
    seed_ = rand();
    ASSERT_NO_FATAL_FAILURE(test_one(seed_, 1024));
    seed_ = rand();
    ASSERT_NO_FATAL_FAILURE(test_one(seed_, 1023));
    seed_ = rand();
    ASSERT_NO_FATAL_FAILURE(test_one(seed_, 750));
    seed_ = rand();
    ASSERT_NO_FATAL_FAILURE(test_one(seed_, 63));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
