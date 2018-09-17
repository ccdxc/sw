// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <gtest/gtest.h>
#include "nic/linkmgr/test/utils/linkmgr_base_test.hpp"
#include "nic/linkmgr/test/utils/port_utils.hpp"

class port_test : public linkmgr_base_test {
protected:
  port_test() {
  }

  virtual ~port_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};

// Create and get
TEST_F(port_test, test1)
{
    int ret    = 0;
    int port   = 1;

    ret = port_create(port);
    ASSERT_TRUE(ret == 0);

    ret = port_get(port, HAL_RET_OK);
    ASSERT_TRUE(ret == 0);
}

// update and get
TEST_F(port_test, test2)
{
    int                ret              = 0;
    int                port             = 1;
    port_speed_t       speed            = port_speed_t::PORT_SPEED_10G;
    port_admin_state_t admin_state      = port_admin_state_t::PORT_ADMIN_STATE_NONE;
    port_fec_type_t    fec_type         = port_fec_type_t::PORT_FEC_TYPE_RS;
    uint32_t           debounce_time    = 100; // ms
    bool               auto_neg_enable  = true;
    port_oper_status_t port_oper_status = port_oper_status_t::PORT_OPER_STATUS_NONE;
    port_type_t        port_type        = port_type_t::PORT_TYPE_NONE;

    ret = port_update(port,
                      HAL_RET_OK,
                      speed,
                      admin_state,
                      fec_type,
                      debounce_time,
                      auto_neg_enable);
    ASSERT_TRUE(ret == 0);

    ret = port_get(port,
                   HAL_RET_OK,
                   true,
                   port_oper_status,
                   port_type,
                   speed,
                   admin_state,
                   fec_type,
                   debounce_time,
                   auto_neg_enable);
    ASSERT_TRUE(ret == 0);
}

// failed update and get
TEST_F(port_test, test3)
{
    int ret  = 0;
    int port = 1;

    // Try configure 100G on 1 lane port
    port_speed_t       speed            = port_speed_t::PORT_SPEED_100G;
    port_admin_state_t admin_state      = port_admin_state_t::PORT_ADMIN_STATE_NONE;
    port_fec_type_t    fec_type         = port_fec_type_t::PORT_FEC_TYPE_RS;
    uint32_t           debounce_time    = 100; // ms
    bool               auto_neg_enable  = true;
    port_oper_status_t port_oper_status = port_oper_status_t::PORT_OPER_STATUS_NONE;
    port_type_t        port_type        = port_type_t::PORT_TYPE_NONE;

    ret = port_update(port,
                      HAL_RET_ERR,
                      speed,
                      admin_state,
                      fec_type,
                      debounce_time,
                      auto_neg_enable);
    ASSERT_TRUE(ret == 0);

    ret = port_get(port,
                   HAL_RET_OK,
                   true,
                   port_oper_status,
                   port_type,
                   port_speed_t::PORT_SPEED_NONE,
                   admin_state,
                   fec_type,
                   debounce_time,
                   auto_neg_enable);
    ASSERT_TRUE(ret == 0);
}

// delete and get
TEST_F(port_test, test4)
{
    int ret    = 0;
    int port   = 1;

    ret = port_delete(port);
    ASSERT_TRUE(ret == 0);

    ret = port_get(port, HAL_RET_PORT_NOT_FOUND);
    ASSERT_TRUE(ret == 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
