#include "nic/hal/src/debug/snake.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "gen/proto/interface.pb.h"
#include "gen/proto/debug.pb.h"
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"

class snake_test : public hal_base_test {
protected:
  snake_test() {
  }

  virtual ~snake_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

  // Will be called at the beginning of all test cases in this class
  static void SetUpTestCase() {
    hal_base_test::SetUpTestCase();
    hal_test_utils_slab_disable_delete();
  }

};

// ----------------------------------------------------------------------------
// Creating a snake test
// ----------------------------------------------------------------------------
TEST_F(snake_test, test1)
{
    hal_ret_t ret;
    LifSpec spec;
    LifResponse resp;
    InterfaceSpec       if_spec;
    InterfaceResponse   if_rsp;
    SnakeTestRequest req;
    SnakeTestResponse rsp;
    SnakeTestDeleteRequest del_req;
    SnakeTestDeleteResponse del_rsp;

    // Uplink if
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 1);
    if_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 2);
    if_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Internal Mgmt Lif
    spec.mutable_key_or_handle()->set_lif_id(69);
    spec.set_name("int_mnic0");
    spec.set_type(types::LIF_TYPE_MNIC_INTERNAL_MANAGEMENT);
    spec.set_is_management(true);
    spec.set_hw_lif_id(69);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(spec, &resp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    req.set_type(types::SNAKE_TEST_TYPE_LOOP);
    req.set_vlan(100);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::snake_test_create(req, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    del_req.set_type(types::SNAKE_TEST_TYPE_LOOP);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::snake_test_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
