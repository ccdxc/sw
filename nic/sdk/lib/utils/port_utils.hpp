// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __SDK_PORT_UTILS_HPP__
#define __SDK_PORT_UTILS_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"

namespace sdk {
namespace lib {

static inline uint32_t
port_speed_enum_to_mbps (port_speed_t speed_enum)
{
    switch (speed_enum) {
    case port_speed_t::PORT_SPEED_1G:
        return 1000;

    case port_speed_t::PORT_SPEED_10G:
        return 10000;

    case port_speed_t::PORT_SPEED_25G:
        return 25000;

    case port_speed_t::PORT_SPEED_40G:
        return 40000;

    case port_speed_t::PORT_SPEED_50G:
        return 50000;

    case port_speed_t::PORT_SPEED_100G:
        return 100000;

    case port_speed_t::PORT_SPEED_NONE:
    default:
        return 0;
    }
}

static inline port_speed_t
port_speed_mbps_to_enum (uint32_t speed)
{
    switch (speed) {
    case 1000:
        return port_speed_t::PORT_SPEED_1G;

    case 10000:
        return port_speed_t::PORT_SPEED_10G;

    case 25000:
        return port_speed_t::PORT_SPEED_25G;

    case 40000:
        return port_speed_t::PORT_SPEED_40G;

    case 50000:
        return port_speed_t::PORT_SPEED_50G;

    case 100000:
        return port_speed_t::PORT_SPEED_100G;

    case 0:
    default:
        return port_speed_t::PORT_SPEED_NONE;
    }
}

static inline uint8_t
port_admin_state_enum_to_uint (port_admin_state_t admin_st)
{
    // TODO @neel move ionic_if.h from platform to sdk and use those enums here
    switch(admin_st) {
    case port_admin_state_t::PORT_ADMIN_STATE_UP:
        return 2;

    case port_admin_state_t::PORT_ADMIN_STATE_DOWN:
    default:
        return 1;
    }
}

static inline port_admin_state_t
port_admin_state_uint_to_enum (uint8_t admin_st)
{
    // TODO @neel move ionic_if.h from platform to sdk and use those enums here
    switch(admin_st) {
    case 2:
        return port_admin_state_t::PORT_ADMIN_STATE_UP;

    case 1:
    default:
        return port_admin_state_t::PORT_ADMIN_STATE_DOWN;
    }
}

static inline uint8_t
port_oper_state_enum_to_uint (port_oper_status_t oper_st)
{
    // TODO @neel move ionic_if.h from platform to sdk and use those enums here
    switch(oper_st) {
    case port_oper_status_t::PORT_OPER_STATUS_UP:
        return 1;

    case port_oper_status_t::PORT_OPER_STATUS_DOWN:
    default:
        return 2;
    }
}

static inline port_oper_status_t
port_oper_state_uint_to_enum (uint8_t oper_st)
{
    // TODO @neel move ionic_if.h from platform to sdk and use those enums here
    switch(oper_st) {
    case 1:
        return port_oper_status_t::PORT_OPER_STATUS_UP;

    case 2:
    default:
        return port_oper_status_t::PORT_OPER_STATUS_DOWN;
    }
}

}    // namespace lib
}    // namespace sdk

#endif // __SDK_PORT_UTILS_HPP__
