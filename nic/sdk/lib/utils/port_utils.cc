// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "port_utils.hpp"

namespace sdk {
namespace lib {

uint32_t
port_speed_enum_to_mbps (port_speed_t speed_enum)
{
    uint32_t speed = 0;

    switch (speed_enum) {
    case port_speed_t::PORT_SPEED_NONE:
        speed = 0;
        break;
    case port_speed_t::PORT_SPEED_1G:
        speed = 1000;
        break;
    case port_speed_t::PORT_SPEED_10G:
        speed = 10000;
        break;
    case port_speed_t::PORT_SPEED_25G:
        speed = 25000;
        break;
    case port_speed_t::PORT_SPEED_40G:
        speed = 40000;
        break;
    case port_speed_t::PORT_SPEED_50G:
        speed = 50000;
        break;
    case port_speed_t::PORT_SPEED_100G:
        speed = 100000;
        break;
    default:
        speed = 0;
    }

    return speed;
}

uint8_t port_admin_state_enum_to_uint (port_admin_state_t admin_st)
{
    switch(admin_st) {
    case port_admin_state_t::PORT_ADMIN_STATE_DOWN:
        return 0;

    case port_admin_state_t::PORT_ADMIN_STATE_UP:
        return 1;

    default:
        return 0;
    }
}

uint8_t port_oper_state_enum_to_uint (port_oper_status_t oper_st)
{
    switch(oper_st) {
    case port_oper_status_t::PORT_OPER_STATUS_DOWN:
        return 0;

    case port_oper_status_t::PORT_OPER_STATUS_UP:
        return 1;

    default:
        return 0;
    }
}
}    // namespace lib
}    // namespace sdk
