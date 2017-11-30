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

bool cb(void *elem, void *data) {
    hal_handle_t *hdl = (hal_handle_t *)elem;
    HAL_TRACE_DEBUG("handle_id: {}", *hdl);
    return true;
}

// test that block_list creation fails with invalid args to constructor
TEST_F(block_list_test, test1) {
    hal_ret_t   ret = HAL_RET_OK;
    block_list *test_block_list;
    hal_handle_t hdl_id = 0, *p_hdl_id = NULL;

    test_block_list = block_list::factory(sizeof(hal_handle_t), 10);
    ASSERT_TRUE(test_block_list != NULL);

    // ret = test_block_list->insert(&hdl_id);
    // ASSERT_TRUE(ret == HAL_RET_OK);
    for (uint32_t i = 0; i < 20; i++) {
        ret = test_block_list->insert(&hdl_id);
        ASSERT_TRUE(ret == HAL_RET_OK);
        hdl_id++;
    }

    hdl_id = 0;
    for (uint32_t i = 0; i < 20; i++) {
        ret = test_block_list->insert(&hdl_id);
        ASSERT_TRUE(ret == HAL_RET_DUP_INS_FAIL);
        hdl_id++;
    }

    // test_block_list->iterate(cb);
    for (auto it = test_block_list->begin(); it != test_block_list->end(); ++it) {
        p_hdl_id = (hal_handle_t *)*it;
        HAL_TRACE_DEBUG("HAL handle: {}", *p_hdl_id);
    }

    for (const void *ptr : *test_block_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        HAL_TRACE_DEBUG("HAL handle: {}", *p_hdl_id);
    }

    // Deleting 9
    auto it = test_block_list->begin();
    while (it != test_block_list->end()) {
        p_hdl_id = (hal_handle_t *)*it;
        if (*p_hdl_id == 9) {
            HAL_TRACE_DEBUG("Delete 9: ");
            test_block_list->erase(it);
        } else {
            ++it;
        }
    }
    HAL_TRACE_DEBUG("After Delete: ");
    for (const void *ptr : *test_block_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        HAL_TRACE_DEBUG("HAL handle: {}", *p_hdl_id);
    }

    // Deleting 18
    auto it1 = test_block_list->begin();
    while (it1 != test_block_list->end()) {
        p_hdl_id = (hal_handle_t *)*it1;
        if (*p_hdl_id == 18) {
            HAL_TRACE_DEBUG("Delete 18: ");
            test_block_list->erase(it1);
        } else {
            ++it1;
        }
    }
    HAL_TRACE_DEBUG("After Delete: ");
    for (const void *ptr : *test_block_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        HAL_TRACE_DEBUG("HAL handle: {}", *p_hdl_id);
    }

    for (uint32_t i = 0; i < 20; i++) {
        hdl_id--;
        if (hdl_id == 9 || hdl_id == 18) {
            continue;
        }
        HAL_TRACE_DEBUG("Trying to remove: {}", hdl_id);
        ret = test_block_list->remove(&hdl_id);
        ASSERT_TRUE(ret == HAL_RET_OK);
        test_block_list->iterate(cb);
    }
    
    block_list::destroy(test_block_list);
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
    block_list::destroy(test_block_list);
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
    block_list::destroy(test_block_list);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
