//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "ftltest_base.hpp"

#define MAX_COUNT     5
#define HASH_VALUE    0xDEADBEEF

class collision: public ftl_test_base {
protected:
    sdk_ret_t remove_collision_from_center(uint32_t count,
                                           sdk_ret_t expret,
                                           bool with_hash = true,
                                           uint32_t hash_32b = 0) {
        int left = count/2;
        int right = count/2 + 1;

        while (left >= 0 || right < count) {
            if (left >= 0) {
                auto rs = remove_helper(left, expret, with_hash, hash_32b);
                MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
                left -= 1;
            }

            if (right < count) {
                auto rs = remove_helper(right, expret, with_hash, hash_32b);
                MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
                right += 1;
            }
        }

        return SDK_RET_OK;
    }

    sdk_ret_t remove_collision_from_head_and_tail(uint32_t count,
                                                  sdk_ret_t expret,
                                                  bool with_hash = true,
                                                  uint32_t hash_32b = 0) {
        int left = 0;
        int right = count-1;

        while (left <= right) {
            if (left == right) {
                auto rs = remove_helper(left, expret, with_hash, hash_32b);
                MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
                left += 1;
                right -= 1;
                break;
            }

            if (left < right) {
                auto rs = remove_helper(left, expret, with_hash, hash_32b);
                MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
                left += 1;
            }

            if (right > left) {
                auto rs = remove_helper(right, expret, with_hash, hash_32b);
                MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
                right -= 1;
            }
        }
        return SDK_RET_OK;
    }

    sdk_ret_t remove_collision_from_tail(uint32_t count,
                                         sdk_ret_t expret,
                                         bool with_hash = true,
                                         uint32_t hash_32b = 0) {
        for (auto i=count-1; i > 0; i--) {
            auto rs = remove_helper(i, expret, with_hash, hash_32b);
            MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        }
        return SDK_RET_OK;
    }
};

TEST_F(collision, insert_full_mesh) {
    sdk_ret_t rs;
    rs = insert(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_update_full_mesh) {
    sdk_ret_t rs;
    rs = insert(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = update(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_remove_full_mesh_iterate) {
    sdk_ret_t rs;
    rs = insert(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = iterate();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = remove(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_full_mesh_more_hashs) {
    sdk_ret_t rs;
    rs = insert(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_remove_from_center) {
    sdk_ret_t rs;
    rs = insert(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = remove_collision_from_center(MAX_COUNT, sdk::SDK_RET_OK,
                                      WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_remove_from_tail) {
    sdk_ret_t rs;
    rs = insert(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = remove_collision_from_tail(MAX_COUNT, sdk::SDK_RET_OK,
                                    WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_remove_from_head_and_tail) {
    sdk_ret_t rs;
    rs = insert(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = remove_collision_from_head_and_tail(MAX_COUNT, sdk::SDK_RET_OK,
                                             WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

