// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_PORT_UTILS_HPP__
#define __SDK_PORT_UTILS_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"

namespace sdk {
namespace lib {

uint32_t port_speed_enum_to_mbps(port_speed_t speed_enum);
uint8_t port_admin_state_enum_to_uint(port_admin_state_t admin_st);
uint8_t port_oper_state_enum_to_uint(port_oper_status_t oper_st);

}    // namespace lib
}    // namespace sdk

#endif // __SDK_PORT_UTILS_HPP__

