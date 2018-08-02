// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __LINKMGR_TEST_PORT_UTILS_HPP__
#define __LINKMGR_TEST_PORT_UTILS_HPP__

#include "nic/include/base.hpp"
#include "nic/gen/proto/hal/port.grpc.pb.h"

using port::Port;
using port::PortSpec;
using port::PortResponse;
using port::PortGetRequest;
using port::PortGetResponse;
using port::PortDeleteRequest;
using port::PortDeleteResponseMsg;

int port_create(uint32_t vrf_id,
                uint32_t port_id,
                types::ApiStatus api_status);

int port_update(uint32_t             vrf_id,
                uint32_t             port_id,
                port::PortSpeed      speed,
                port::PortAdminState admin_state,
                port::PortFecType    fec_type,
                uint32_t             debounce_time,
                bool                 auto_neg_enable,
                types::ApiStatus     api_status);

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
        bool                 auto_neg_enable  = false);

int port_delete(uint32_t vrf_id, uint32_t port_id, types::ApiStatus api_status);

#endif // __LINKMGR_TEST_PORT_UTILS_HPP__
