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
    int vrf_id = 1;
    int port   = 1;

    ret = port_create(vrf_id,
                      port,
                      types::ApiStatus::API_STATUS_OK);
    ASSERT_TRUE(ret == 0);

    ret = port_get(vrf_id,
                   port,
                   types::ApiStatus::API_STATUS_OK,
                   HAL_RET_OK);
    ASSERT_TRUE(ret == 0);
}

// update and get
TEST_F(port_test, test2)
{
    int                  ret              = 0;
    int                  vrf_id           = 1;
    int                  port             = 1;
    port::PortSpeed      speed            = ::port::PORT_SPEED_10G;
    port::PortAdminState admin_state      = ::port::PORT_ADMIN_STATE_NONE;
    port::PortFecType    fec_type         = ::port::PORT_FEC_TYPE_RS;
    uint32_t             debounce_time    = 100; // ms
    bool                 auto_neg_enable  = true;
    types::ApiStatus     api_status       = types::ApiStatus::API_STATUS_OK;
    port::PortOperStatus port_oper_status = port::PORT_OPER_STATUS_NONE;
    port::PortType       port_type        = port::PORT_TYPE_NONE;

    ret = port_update(vrf_id,
                      port,
                      speed,
                      admin_state,
                      fec_type,
                      debounce_time,
                      auto_neg_enable,
                      api_status);
    ASSERT_TRUE(ret == 0);

    ret = port_get(vrf_id,
                   port,
                   types::ApiStatus::API_STATUS_OK,
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

// delete and get
TEST_F(port_test, test3)
{
    int ret    = 0;
    int vrf_id = 1;
    int port   = 1;

    ret = port_delete(vrf_id, port, types::ApiStatus::API_STATUS_OK);
    ASSERT_TRUE(ret == 0);

    ret = port_get(vrf_id,
                   port,
                   types::ApiStatus::API_STATUS_NOT_FOUND,
                   HAL_RET_PORT_NOT_FOUND);
    ASSERT_TRUE(ret == 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
