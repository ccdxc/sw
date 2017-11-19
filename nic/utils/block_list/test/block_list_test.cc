#include "nic/utils/block_list/block_list.hpp"
#include <gtest/gtest.h>

using hal::utils::block_list;

class block_list_test : public ::testing::Test {
protected:
    block_list_test() {
    }

    virtual ~block_list_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

void cb(void *elem) {
    hal_handle_t *hdl = (hal_handle_t *)elem;
    HAL_TRACE_DEBUG("handle_id: {}", *hdl);
}

// test that block_list creation fails with invalid args to constructor
TEST_F(block_list_test, test1) {
    hal_ret_t   ret = HAL_RET_OK;
    block_list *test_block_list;
    hal_handle_t hdl_id = 0;

    test_block_list = block_list::factory(sizeof(hal_handle_t), 10);
    ASSERT_TRUE(test_block_list != NULL);

    // ret = test_block_list->insert(&hdl_id);
    // ASSERT_TRUE(ret == HAL_RET_OK);
    for (uint32_t i = 0; i < 20; i++) {
        ret = test_block_list->insert(&hdl_id);
        ASSERT_TRUE(ret == HAL_RET_OK);
        hdl_id++;
    }

    test_block_list->iterate(cb);

    for (uint32_t i = 0; i < 20; i++) {
        hdl_id--;
        HAL_TRACE_DEBUG("Trying to remove: {}", hdl_id);
        ret = test_block_list->remove(&hdl_id);
        ASSERT_TRUE(ret == HAL_RET_OK);
        test_block_list->iterate(cb);
    }
    
    delete test_block_list;
}

TEST_F(block_list_test, test2) {
    hal_ret_t   ret = HAL_RET_OK;
    block_list *test_block_list;
    hal_handle_t hdl_id = 0;

    test_block_list = block_list::factory(sizeof(hal_handle_t), 10);
    ASSERT_TRUE(test_block_list != NULL);

    for (uint32_t i = 0; i < 20; i++) {
        ret = test_block_list->insert(&hdl_id);
        ASSERT_TRUE(ret == HAL_RET_OK);
        hdl_id++;
    }
    test_block_list->iterate(cb);
    hdl_id = 0;
    for (uint32_t i = 0; i < 20; i++) {
        HAL_TRACE_DEBUG("Trying to remove: {}", hdl_id);
        ret = test_block_list->remove(&hdl_id);
        ASSERT_TRUE(ret == HAL_RET_OK);
        test_block_list->iterate(cb);
        hdl_id++;
    }
    delete test_block_list;
}

TEST_F(block_list_test, test3) {
    hal_ret_t   ret = HAL_RET_OK;
    block_list *test_block_list;
    hal_handle_t hdl_id = 0;

    test_block_list = block_list::factory(sizeof(hal_handle_t), 15);
    ASSERT_TRUE(test_block_list != NULL);

    for (uint32_t i = 0; i < 50; i++) {
        ret = test_block_list->insert(&hdl_id);
        ASSERT_TRUE(ret == HAL_RET_OK);
        hdl_id++;
    }
    test_block_list->iterate(cb);
    hdl_id = 0;
    for (uint32_t i = 0; i < 25; i++) {
        HAL_TRACE_DEBUG("Trying to remove: {}", hdl_id);
        ret = test_block_list->remove(&hdl_id);
        ASSERT_TRUE(ret == HAL_RET_OK);
        test_block_list->iterate(cb);
        hdl_id++;
    }

    hdl_id = 0;
    for (uint32_t i = 0; i < 25; i++) {
        ret = test_block_list->insert(&hdl_id);
        ASSERT_TRUE(ret == HAL_RET_OK);
        hdl_id++;
    }

    hdl_id = 0;
    for (uint32_t i = 0; i < 50; i++) {
        HAL_TRACE_DEBUG("Trying to remove: {}", hdl_id);
        ret = test_block_list->remove(&hdl_id);
        ASSERT_TRUE(ret == HAL_RET_OK);
        test_block_list->iterate(cb);
        hdl_id++;
    }
    delete test_block_list;
}
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
