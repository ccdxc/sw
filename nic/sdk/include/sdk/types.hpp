//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// SDK types header file
//------------------------------------------------------------------------------

#ifndef __SDK_TYPES_HPP__
#define __SDK_TYPES_HPP__

namespace sdk {
namespace types {

enum class port_speed_t {
    PORT_SPEED_NONE    = 0,
    PORT_SPEED_10G     = 1,
    PORT_SPEED_25G     = 2,
    PORT_SPEED_100G    = 3,
};

enum class port_type_t {
    PORT_TYPE_NONE    = 0,
    PORT_TYPE_ETH     = 1,
};

enum class platform_type_t {
    PLATFORM_TYPE_NONE = 0,
    PLATFORM_TYPE_SIM  = 1,
    PLATFORM_TYPE_HAPS = 2,
    PLATFORM_TYPE_HW   = 3,
};

enum class port_admin_state_t {
    PORT_ADMIN_STATE_NONE    = 0,
    PORT_ADMIN_STATE_UP      = 1,
    PORT_ADMIN_STATE_DOWN    = 2,
};

enum class port_oper_status_t {
    PORT_OPER_STATUS_NONE = 0,
    PORT_OPER_STATUS_UP = 1,
    PORT_OPER_STATUS_DOWN = 2,
};

}    // namespace types
}    // namespace sdk

using sdk::types::port_speed_t;
using sdk::types::port_type_t;
using sdk::types::port_admin_state_t;
using sdk::types::port_oper_status_t;
using sdk::types::platform_type_t;

#endif    // __SDK_TYPES_HPP__

