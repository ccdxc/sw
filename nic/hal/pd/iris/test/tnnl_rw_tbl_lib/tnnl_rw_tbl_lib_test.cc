#include <gtest/gtest.h>
#include "nic/hal/pd/iris/tnnl_rw_pd.hpp"
#include "nic/include/hal_pd.hpp"
#include "nic/include/eth.h"

using hal::pd::pd_tnnl_rw_entry_key_t;
using hal::pd::pd_tnnl_rw_entry_t;

class tnnl_rw_test : public ::testing::Test {
protected:
    tnnl_rw_test() {
  }

  virtual ~tnnl_rw_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
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
TEST_F(tnnl_rw_test, test1) {
    hal_ret_t           ret;
    pd_tnnl_rw_entry_key_t   key;
    pd_tnnl_rw_entry_t       *tnnl_rwe;
    uint64_t            mac_sa = 0x000000000001;
    uint64_t            mac_da = 0x000000000002;
    uint64_t            mac_da1 = 0x000000000003;
    uint32_t            tnnl_rw_idx = 0, tnnl_rw_idx1 = 0;

    MAC_UINT64_TO_ADDR(key.mac_sa, mac_sa);
    MAC_UINT64_TO_ADDR(key.mac_da, mac_da);
    key.tnnl_rw_act = TUNNEL_REWRITE_ENCAP_VXLAN_ID;

    ret = tnnl_rw_entry_find(&key, &tnnl_rwe);
    ASSERT_TRUE(ret == HAL_RET_ENTRY_NOT_FOUND);

    ret = tnnl_rw_entry_alloc(&key, NULL, &tnnl_rw_idx);
    ASSERT_TRUE(ret == HAL_RET_OK);

    ret = tnnl_rw_entry_find_or_alloc(&key, &tnnl_rw_idx1);
    ASSERT_TRUE(ret == HAL_RET_OK);

    ASSERT_TRUE(tnnl_rw_idx == tnnl_rw_idx1);

    MAC_UINT64_TO_ADDR(key.mac_da, mac_da1);
    ret = tnnl_rw_entry_alloc(&key, NULL, &tnnl_rw_idx);
    ASSERT_TRUE(ret == HAL_RET_OK);
    
    MAC_UINT64_TO_ADDR(key.mac_da, mac_da);
    ret = tnnl_rw_entry_delete(&key);
    ASSERT_TRUE(ret == HAL_RET_OK);

    ret = tnnl_rw_entry_delete(&key);
    ASSERT_TRUE(ret == HAL_RET_OK);

    ret = tnnl_rw_entry_delete(&key);
    ASSERT_TRUE(ret == HAL_RET_ENTRY_NOT_FOUND);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  hal::pd::hal_pd_mem_init();
  return RUN_ALL_TESTS();
}
