//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// SDK types header file
//------------------------------------------------------------------------------

#ifndef __SDK_TYPES_HPP__
#define __SDK_TYPES_HPP__

#include <inttypes.h>

namespace sdk {
namespace types {

typedef uint64_t    hbm_addr_t;
typedef uint64_t    mem_addr_t;

enum class port_event_t {
    PORT_EVENT_LINK_NONE = 0,
    PORT_EVENT_LINK_UP   = 1,
    PORT_EVENT_LINK_DOWN = 2
};

typedef enum cable_type_e {
    CABLE_TYPE_CU,
    CABLE_TYPE_FIBER,
    CABLE_TYPE_MAX
} cable_type_t;

typedef enum port_breakout_mode_e {
    BREAKOUT_MODE_NONE,
    BREAKOUT_MODE_4x25G,
    BREAKOUT_MODE_4x10G,
    BREAKOUT_MODE_2x50G
} port_breakout_mode_t;

typedef enum q_notify_mode_e {
    Q_NOTIFY_MODE_BLOCKING     = 0,
    Q_NOTIFY_MODE_NON_BLOCKING = 1
} q_notify_mode_t;

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
    PLATFORM_TYPE_RTL = 6,
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

enum class port_pause_type_t {
    PORT_PAUSE_TYPE_NONE,  // Disable pause
    PORT_PAUSE_TYPE_LINK,  // Link level pause
    PORT_PAUSE_TYPE_PFC,   // PFC
};

typedef enum xcvr_state_s {
    XCVR_REMOVED,
    XCVR_INSERTED,
    XCVR_SPROM_PENDING,
    XCVR_SPROM_READ,
    XCVR_SPROM_READ_ERR,
} xcvr_state_t;

typedef enum xcvr_type_s {
    XCVR_TYPE_UNKNOWN,
    XCVR_TYPE_SFP,
    XCVR_TYPE_QSFP,
    XCVR_TYPE_QSFP28,
} xcvr_type_t;

typedef enum xcvr_pid_s {
    XCVR_PID_UNKNOWN,
    XCVR_PID_SFP_10G_CR,
    XCVR_PID_QSFP_100G_CR4,
} xcvr_pid_t;

}    // namespace types
}    // namespace sdk

using sdk::types::port_speed_t;
using sdk::types::port_type_t;
using sdk::types::port_admin_state_t;
using sdk::types::port_oper_status_t;
using sdk::types::platform_type_t;
using sdk::types::port_fec_type_t;
using sdk::types::port_pause_type_t;
using sdk::types::port_event_t;
using sdk::types::port_breakout_mode_t;
using sdk::types::mem_addr_t;
using sdk::types::hbm_addr_t;
using sdk::types::q_notify_mode_t;

#endif    // __SDK_TYPES_HPP__

