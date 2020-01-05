//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "ftltest_base.hpp"

class collision: public FtlGtestBase {
protected:
    /**
     * Remove collision list starting from center element.
     */
    sdk_ret_t RemoveCollisionFromCenter(uint32_t count,
                                        sdk_ret_t expret,
                                        bool with_hash = true,
                                        uint32_t hash_32b = 0) {
        int left = count/2;
        int right = count/2 + 1;

        while (left >= 0 || right < count) {
            if (left >= 0) {
                auto rs = RemoveHelper(left, expret, with_hash, hash_32b);
                MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
                left -= 1;
            }

            if (right < count) {
                auto rs = RemoveHelper(right, expret, with_hash, hash_32b);
                MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
                right += 1;
            }
        }

        return SDK_RET_OK;
    }

    /**
     * Remove collision list starting from left and right end.
     */
    sdk_ret_t RemoveCollisionFromLeftAndRightEnd(uint32_t count,
                                                 sdk_ret_t expret,
                                                 bool with_hash = true,
                                                 uint32_t hash_32b = 0) {
        int left = 0;
        int right = count-1;

        while (left <= right) {
            if (left == right) {
                auto rs = RemoveHelper(left, expret, with_hash, hash_32b);
                MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
                left += 1;
                right -= 1;
                break;
            }

            if (left < right) {
                auto rs = RemoveHelper(left, expret, with_hash, hash_32b);
                MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
                left += 1;
            }

            if (right > left) {
                auto rs = RemoveHelper(right, expret, with_hash, hash_32b);
                MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
                right -= 1;
            }
        }
        return SDK_RET_OK;
    }

    /**
     * Removes collision list from end.
     */
    sdk_ret_t RemoveCollisionFromRightEnd(uint32_t count,
                                          sdk_ret_t expret,
                                          bool with_hash = true,
                                          uint32_t hash_32b = 0) {
        for (auto i=count-1; i >= 0; i--) {
            auto rs = RemoveHelper(i, expret, with_hash, hash_32b);
            MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        }
        return SDK_RET_OK;
    }
};

#define MAX_COUNT     5
#define HASH_VALUE    0xDEADBEEF

TEST_F(collision, insert_full_mesh) {
    sdk_ret_t rs;
    rs = Insert(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_update_full_mesh) {
    sdk_ret_t rs;
    rs = Insert(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = Update(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_remove_full_mesh_iterate) {
    sdk_ret_t rs;
    rs = Insert(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = Iterate();
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = Remove(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_full_mesh_more_hashs) {
    sdk_ret_t rs;
    rs = Insert(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_remove_from_center) {
    sdk_ret_t rs;
    rs = Insert(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = RemoveCollisionFromCenter(MAX_COUNT, sdk::SDK_RET_OK,
                                   WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_remove_from_end) {
    sdk_ret_t rs;
    rs = Insert(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = RemoveCollisionFromLeftAndRightEnd(MAX_COUNT, sdk::SDK_RET_OK,
                                            WITH_HASH, HASH_VALUE);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

#if 0
TEST_F(collision, insert_remove_full_mesh_more_hashs) {
    sdk_ret_t rs;
    rs = InsertWithHash(MAX_MORE_LEVELS, MAX_HINTS, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = RemoveAllCached(sdk::SDK_RET_OK, false);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, insert_update_full_mesh_more_hashs) {
    sdk_ret_t rs;
    rs = InsertWithHash(MAX_MORE_LEVELS, MAX_HINTS, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = UpdateAllCached(sdk::SDK_RET_OK, false);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, exm_match_chain) {
    sdk_ret_t rs;
    
    rs = InsertWithHash(1, 1, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, update_exm_match_chain) {
    sdk_ret_t rs;
    
    rs = InsertWithHash(1, 1, MAX_RECIRCS);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = UpdateAllCached(sdk::SDK_RET_OK, false);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(collision, max_recirc) {
    sdk_ret_t rs;
    rs = InsertWithHash(1, 1, MAX_RECIRCS + 1);
    ASSERT_TRUE(rs == sdk::SDK_RET_MAX_RECIRC_EXCEED);
}
#endif
