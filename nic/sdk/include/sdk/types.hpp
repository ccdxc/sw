//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// SDK types header file
//------------------------------------------------------------------------------

#ifndef __SDK_TYPES_HPP__
#define __SDK_TYPES_HPP__

namespace sdk {
namespace types {

enum class port_event_t {
    PORT_EVENT_LINK_UP   = 0,
    PORT_EVENT_LINK_DOWN = 1
};

typedef enum cable_type_e {
    CABLE_TYPE_CU,
    CABLE_TYPE_FIBER,
    CABLE_TYPE_MAX
} cable_type_t;

enum class port_speed_t {
    PORT_SPEED_NONE    = 0,
    PORT_SPEED_1G      = 1,
    PORT_SPEED_10G     = 2,
    PORT_SPEED_25G     = 3,
    PORT_SPEED_40G     = 4,
    PORT_SPEED_50G     = 5,
    PORT_SPEED_100G    = 6,
    PORT_SPEED_MAX     = 7,
};

enum class port_type_t {
    PORT_TYPE_NONE    = 0,
    PORT_TYPE_ETH     = 1,
    PORT_TYPE_MGMT    = 2,
};

enum class platform_type_t {
    PLATFORM_TYPE_NONE = 0,
    PLATFORM_TYPE_SIM  = 1,
    PLATFORM_TYPE_HAPS = 2,
    PLATFORM_TYPE_HW   = 3,
    PLATFORM_TYPE_MOCK = 4,
    PLATFORM_TYPE_ZEBU = 5,
};

enum class port_admin_state_t {
    PORT_ADMIN_STATE_NONE    = 0,
    PORT_ADMIN_STATE_UP      = 1,
    PORT_ADMIN_STATE_DOWN    = 2,
};

enum class port_oper_status_t {
    PORT_OPER_STATUS_NONE = 0,
    PORT_OPER_STATUS_UP   = 1,
    PORT_OPER_STATUS_DOWN = 2,
};

enum class port_fec_type_t {
    PORT_FEC_TYPE_NONE,  // Disable FEC
    PORT_FEC_TYPE_FC,    // Enable FireCode FEC
    PORT_FEC_TYPE_RS,    // Enable ReedSolomon FEC
};

}    // namespace types
}    // namespace sdk

using sdk::types::port_speed_t;
using sdk::types::port_type_t;
using sdk::types::port_admin_state_t;
using sdk::types::port_oper_status_t;
using sdk::types::platform_type_t;
using sdk::types::port_fec_type_t;
using sdk::types::port_event_t;

#endif    // __SDK_TYPES_HPP__

