#include <gtest/gtest.h>
#include "nic/hal/pd/iris/rw_pd.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/include/hal_pd.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/include/eth.h"

using hal::pd::pd_rw_entry_key_t;
using hal::pd::pd_rw_entry_t;

class rw_test : public ::testing::Test {
protected:
    rw_test() {
  }

  virtual ~rw_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

  static void SetUpTestCase() {
      hal::g_delay_delete = false;
  }

};

/* -----------------------------------------------------------------------------
 *
 * Test Case 1:
 *      - Test Case to verify the insert
 * - Create DM table
 * - Insert DM Entry
 *
 * ---------------------------------------------------------------------------*/
TEST_F(rw_test, test1) {
    hal_ret_t           ret;
    pd_rw_entry_key_t   key;
    pd_rw_entry_t       *rwe;
    uint64_t            mac_sa = 0x000000000001;
    uint64_t            mac_da = 0x000000000002;
    uint64_t            mac_da1 = 0x000000000003;
    uint32_t            rw_idx = 0, rw_idx1 = 0;

    MAC_UINT64_TO_ADDR(key.mac_sa, mac_sa);
    MAC_UINT64_TO_ADDR(key.mac_da, mac_da);
    key.rw_act = (hal::rewrite_actions_en)REWRITE_REWRITE_ID;

    ret = rw_entry_find(&key, &rwe);
    ASSERT_TRUE(ret == HAL_RET_ENTRY_NOT_FOUND);

    ret = rw_entry_alloc(&key, NULL, &rw_idx);
    ASSERT_TRUE(ret == HAL_RET_OK);

    ret = rw_entry_find_or_alloc(&key, &rw_idx1);
    ASSERT_TRUE(ret == HAL_RET_OK);

    ASSERT_TRUE(rw_idx == rw_idx1);

    MAC_UINT64_TO_ADDR(key.mac_da, mac_da1);
    ret = rw_entry_alloc(&key, NULL, &rw_idx);
    ASSERT_TRUE(ret == HAL_RET_OK);
    
    MAC_UINT64_TO_ADDR(key.mac_da, mac_da);
    ret = rw_entry_delete(&key);
    ASSERT_TRUE(ret == HAL_RET_OK);

    ret = rw_entry_delete(&key);
    ASSERT_TRUE(ret == HAL_RET_OK);

    ret = rw_entry_delete(&key);
    ASSERT_TRUE(ret == HAL_RET_ENTRY_NOT_FOUND);
}

#if 0
TEST_F(rw_test, test1) {

    output_mapping_actiondata dm;
    uint32_t index;
    dm.actionid = 1;

    hal_ret_t rt;
    rt = test_dm.insert(&dm, &index); 
    ASSERT_TRUE(rt == HAL_RET_OK);

}
#endif

int main(int argc, char **argv) {
    hal::pd::pd_mem_init_args_t    args;

    ::testing::InitGoogleTest(&argc, argv);
    args.cfg_path = std::getenv("HAL_CONFIG_PATH");
    hal::pd::pd_mem_init(&args);
    return RUN_ALL_TESTS();
}
