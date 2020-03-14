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

static inline sdk_ret_t
port_speed_mbps_to_enum (uint32_t speed, port_speed_t *port_speed)
{
    switch (speed) {
    case 1000:
        *port_speed = port_speed_t::PORT_SPEED_1G;
        break;

    case 10000:
        *port_speed = port_speed_t::PORT_SPEED_10G;
        break;

    case 25000:
        *port_speed = port_speed_t::PORT_SPEED_25G;
        break;

    case 40000:
        *port_speed = port_speed_t::PORT_SPEED_40G;
        break;

    case 50000:
        *port_speed = port_speed_t::PORT_SPEED_50G;
        break;

    case 100000:
        *port_speed = port_speed_t::PORT_SPEED_100G;
        break;

    case 0:
    default:
        SDK_TRACE_ERR("Invalid speed %u", speed);
        *port_speed = port_speed_t::PORT_SPEED_NONE;
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
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

// \@brief       Get the number of lanes based on port speed.
//               Applicable only for NRZ serdes
//               100G/40G   : num_lanes = 4
//               50G        : num_lanes = 2
//               25G/10G/1G : num_lanes = 1
// \@param[in]   speed port speed
// \@params[out] num_lanes number of lanes based on speed
// \@returns     SDK_RET_OK on success, SDK_RET_ERR on failure
static inline sdk_ret_t
port_num_lanes_speed (uint32_t speed, uint32_t *num_lanes)
{
    switch (speed) {
    case 100000:
    case 40000:
        *num_lanes = 4;
        break;

    case 50000:
        *num_lanes = 2;
        break;

    case 25000:
    case 10000:
    case 1000:
        *num_lanes = 1;
        break;

    case 0:
    default:
        SDK_TRACE_ERR("Invalid speed {}", speed);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

}    // namespace lib
}    // namespace sdk

#endif // __SDK_PORT_UTILS_HPP__
