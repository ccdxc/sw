#include <stdio.h>
#include <stdlib.h>
#include <gtest/gtest.h>
#include "nic/include/base.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/port.grpc.pb.h"
#include "nic/linkmgr/test/utils/linkmgr_base_test.hpp"

using port::Port;
using port::PortSpec;
using port::PortResponse;
using port::PortGetRequest;
using port::PortGetResponse;
using port::PortDeleteRequest;
using port::PortDeleteResponseMsg;

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

int port_create(uint32_t vrf_id,
                uint32_t port_id,
                types::ApiStatus api_status)
{
    hal_ret_t     ret;
    PortSpec      spec;
    PortResponse  rsp;

    spec.Clear();
    spec.mutable_key_or_handle()->set_port_id(port_id);
    spec.mutable_meta()->set_vrf_id(vrf_id);
    spec.set_port_speed(::port::PORT_SPEED_25G);
    spec.set_num_lanes(1);
    spec.set_port_type(::port::PORT_TYPE_ETH);
    spec.set_admin_state(::port::PORT_ADMIN_STATE_UP);

    g_linkmgr_state->cfg_db_open(hal::CFG_OP_WRITE);

    ret = linkmgr::port_create(spec, &rsp);

    g_linkmgr_state->cfg_db_close();

    if (ret != HAL_RET_OK) {
        std::cout << __func__ << ": failed" << std::endl;
        return -1;
    }

    if (rsp.api_status() != api_status) {
        std::cout << __func__ << ": API failed" << std::endl;
        return -1;
    }

    spec.Clear();
    return 0;
}

int port_update(uint32_t             vrf_id,
                uint32_t             port_id,
                port::PortSpeed      speed,
                port::PortAdminState admin_state,
                port::PortFecType    fec_type,
                uint32_t             debounce_time,
                bool                 auto_neg_enable,
                types::ApiStatus     api_status)
{
    hal_ret_t     ret;
    PortSpec      spec;
    PortResponse  rsp;

    spec.Clear();
    spec.mutable_key_or_handle()->set_port_id(port_id);
    spec.mutable_meta()->set_vrf_id(vrf_id);
    spec.set_port_speed(speed);
    spec.set_admin_state(admin_state);
    spec.set_fec_type(fec_type);
    spec.set_debounce_time(debounce_time);
    spec.set_auto_neg_enable(auto_neg_enable);

    g_linkmgr_state->cfg_db_open(hal::CFG_OP_WRITE);
    ret = linkmgr::port_update(spec, &rsp);
    g_linkmgr_state->cfg_db_close();
    if (ret != HAL_RET_OK) {
        std::cout << __func__ << ": failed" << std::endl;
        return -1;
    }
    if (rsp.api_status() != api_status) {
        std::cout << __func__ << ": API failed" << std::endl;
        return -1;
    }
    spec.Clear();
    return 0;
}

int port_get(
        uint32_t             vrf_id,
        uint32_t             port_id,
        types::ApiStatus     api_status,
        hal_ret_t            ret_exp,
        bool                 compare          = false,
        port::PortOperStatus port_oper_status = port::PORT_OPER_STATUS_NONE,
        port::PortType       port_type        = port::PORT_TYPE_NONE,
        port::PortSpeed      port_speed       = port::PORT_SPEED_NONE,
        port::PortAdminState port_admin_state = port::PORT_ADMIN_STATE_NONE,
        port::PortFecType    fec_type         = port::PORT_FEC_TYPE_NONE,
        uint32_t             debounce_time    = 100,
        bool                 auto_neg_enable  = false)
{
    hal_ret_t     ret;
    PortGetRequest   req;
    PortGetResponse  rsp;

    req.Clear();
    req.mutable_key_or_handle()->set_port_id(port_id);
    req.mutable_meta()->set_vrf_id(vrf_id);

    g_linkmgr_state->cfg_db_open(hal::CFG_OP_READ);
    ret = linkmgr::port_get(req, &rsp);
    g_linkmgr_state->cfg_db_close();
    if (ret != ret_exp) {
        std::cout << __func__ << ": failed" << std::endl;
        return -1;
    }
    if (rsp.api_status() != api_status) {
        std::cout << __func__ << ": API failed" << std::endl;
        return -1;
    }
    req.Clear();

    std::cout << "Port Get succeeded for port "
              << port_id << std::endl
              << " Port oper status: "
              << rsp.status().oper_status() << std::endl
              << " Port type: "
              << rsp.spec().port_type() << std::endl
              << " Admin state: "
              << rsp.spec().admin_state() << std::endl
              << " Port speed: "
              << rsp.spec().port_speed() << std::endl
              << " MAC ID: "
              << rsp.spec().mac_id() << std::endl
              << " MAC channel: "
              << rsp.spec().mac_ch() << std::endl
              << " Num lanes: "
              << rsp.spec().num_lanes() << std::endl;

    if (compare == true) {
        if (port_oper_status != port::PORT_OPER_STATUS_NONE) {
            if (rsp.status().oper_status() != port_oper_status) {
                std::cout << __func__
                          << ": oper_status get does not match."
                          << " Expected: "
                          << port_oper_status
                          << ", Got: "
                          << rsp.status().oper_status()
                          << std::endl;
                return -1;
            }
        }

        if (port_type != port::PORT_TYPE_NONE) {
            if (rsp.spec().port_type() != port_type) {
                std::cout << __func__
                          << ": port_type get response does not match."
                          << " Expected: "
                          << port_type
                          << ", Got: "
                          << rsp.spec().port_type()
                          << std::endl;
                return -1;
            }
        }

        if (port_speed != port::PORT_SPEED_NONE) {
            if (rsp.spec().port_speed() != port_speed) {
                std::cout << __func__
                          << ": port_speed get response does not match"
                          << " Expected: "
                          << port_speed
                          << ", Got: "
                          << rsp.spec().port_speed()
                          << std::endl;
                return -1;
            }
        }

        if (port_admin_state != port::PORT_ADMIN_STATE_NONE) {
            if (rsp.spec().admin_state() != port_admin_state) {
                std::cout << __func__
                          << ": admin_state get response does not match"
                          << " Expected: "
                          << port_admin_state
                          << ", Got: "
                          << rsp.spec().admin_state()
                          << std::endl;
                return -1;
            }
        }

        if (rsp.spec().fec_type() != fec_type) {
            std::cout << __func__
                      << ": fec_type get response does not match"
                      << " Expected: "
                      << fec_type
                      << ", Got: "
                      << rsp.spec().fec_type()
                      << std::endl;
            return -1;
        }

        if (rsp.spec().debounce_time() != debounce_time) {
            std::cout << __func__
                      << ": debounce_time get response does not match"
                      << " Expected: "
                      << debounce_time
                      << ", Got: "
                      << rsp.spec().debounce_time()
                      << std::endl;
            return -1;
        }

        if (rsp.spec().auto_neg_enable() != auto_neg_enable) {
            std::cout << __func__
                      << ": AutoNeg get response does not match"
                      << " Expected: "
                      << auto_neg_enable
                      << ", Got: "
                      << rsp.spec().auto_neg_enable()
                      << std::endl;
            return -1;
        }
    }
    return 0;
}

int port_delete(uint32_t vrf_id, uint32_t port_id, types::ApiStatus api_status) {
    hal_ret_t           ret;
    PortDeleteRequest   req;
    PortDeleteResponseMsg  rsp;

    req.Clear();
    req.mutable_key_or_handle()->set_port_id(port_id);
    req.mutable_meta()->set_vrf_id(vrf_id);

    g_linkmgr_state->cfg_db_open(hal::CFG_OP_WRITE);
    ret = linkmgr::port_delete(req, &rsp);
    g_linkmgr_state->cfg_db_close();
    if (ret != HAL_RET_OK) {
        std::cout << __func__ << ": failed" << std::endl;
        return -1;
    }
    if (rsp.response(0).api_status() != api_status) {
        std::cout << __func__ << ": API failed" << std::endl;
        return -1;
    }
    req.Clear();
    return 0;
}

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
