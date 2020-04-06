//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// SDK types header file
//------------------------------------------------------------------------------

#ifndef __SDK_TYPES_HPP__
#define __SDK_TYPES_HPP__

#include <cinttypes>
#include <string>

#define XCVR_SPROM_SIZE 256

namespace sdk {
namespace types {

typedef uint64_t    mem_addr_t;

typedef enum monitor_type_e {
    MONITOR_TYPE_SYSTEM     = 0,
    MONITOR_TYPE_INTERRUPTS = 1,
} monitor_type_t;

enum class port_link_sm_t {
    PORT_LINK_SM_DISABLED,
    PORT_LINK_SM_ENABLED,

    PORT_LINK_SM_AN_CFG,
    PORT_LINK_SM_AN_DISABLED,
    PORT_LINK_SM_AN_START,
    PORT_LINK_SM_AN_WAIT_HCD,
    PORT_LINK_SM_AN_COMPLETE,

    PORT_LINK_SM_SERDES_CFG,
    PORT_LINK_SM_WAIT_SERDES_RDY,
    PORT_LINK_SM_MAC_CFG,
    PORT_LINK_SM_SIGNAL_DETECT,

    PORT_LINK_SM_AN_DFE_TUNING,
    PORT_LINK_SM_DFE_TUNING,
    PORT_LINK_SM_DFE_DISABLED,
    PORT_LINK_SM_DFE_START_ICAL,
    PORT_LINK_SM_DFE_WAIT_ICAL,
    PORT_LINK_SM_DFE_START_PCAL,
    PORT_LINK_SM_DFE_WAIT_PCAL,
    PORT_LINK_SM_DFE_PCAL_CONTINUOUS,

    PORT_LINK_SM_CLEAR_MAC_REMOTE_FAULTS,
    PORT_LINK_SM_WAIT_MAC_SYNC,
    PORT_LINK_SM_WAIT_MAC_FAULTS_CLEAR,
    PORT_LINK_SM_UP
};

static inline std::string
port_link_sm_to_str (port_link_sm_t state) {
    switch (state) {
    case port_link_sm_t::PORT_LINK_SM_DISABLED:
        return "DISABLED";
    case port_link_sm_t::PORT_LINK_SM_ENABLED:
        return "ENABLED";
    case port_link_sm_t::PORT_LINK_SM_AN_CFG:
        return "AN_CFG";
    case port_link_sm_t::PORT_LINK_SM_AN_DISABLED:
        return "AN_DISABLED";
    case port_link_sm_t::PORT_LINK_SM_AN_START:
        return "AN_START";
    case port_link_sm_t::PORT_LINK_SM_AN_WAIT_HCD:
        return "AN_WAIT_HCD";
    case port_link_sm_t::PORT_LINK_SM_AN_COMPLETE:
        return "AN_COMPLETE";
    case port_link_sm_t::PORT_LINK_SM_SERDES_CFG:
        return "SERDES_CFG";
    case port_link_sm_t::PORT_LINK_SM_WAIT_SERDES_RDY:
        return "WAIT_SERDES_RDY";
    case port_link_sm_t::PORT_LINK_SM_MAC_CFG:
        return "MAC_CFG";
    case port_link_sm_t::PORT_LINK_SM_SIGNAL_DETECT:
        return "SIGNAL_DETECT";
    case port_link_sm_t::PORT_LINK_SM_AN_DFE_TUNING:
        return "AN_DFE_TUNING";
    case port_link_sm_t::PORT_LINK_SM_DFE_TUNING:
        return "DFE_TUNING";
    case port_link_sm_t::PORT_LINK_SM_DFE_DISABLED:
        return "DFE_DISABLED";
    case port_link_sm_t::PORT_LINK_SM_DFE_START_ICAL:
        return "DFE_START_ICAL";
    case port_link_sm_t::PORT_LINK_SM_DFE_WAIT_ICAL:
        return "DFE_WAIT_ICAL";
    case port_link_sm_t::PORT_LINK_SM_DFE_START_PCAL:
        return "DFE_START_PCAL";
    case port_link_sm_t::PORT_LINK_SM_DFE_WAIT_PCAL:
        return "DFE_WAIT_PCAL";
    case port_link_sm_t::PORT_LINK_SM_DFE_PCAL_CONTINUOUS:
        return "DFE_PCAL_CONTINUOUS";
    case port_link_sm_t::PORT_LINK_SM_CLEAR_MAC_REMOTE_FAULTS:
        return "CLEAR_MAC_REMOTE_FAULTS";
    case port_link_sm_t::PORT_LINK_SM_WAIT_MAC_SYNC:
        return "WAIT_MAC_SYNC";
    case port_link_sm_t::PORT_LINK_SM_WAIT_MAC_FAULTS_CLEAR:
        return "WAIT_MAC_FAULTS_CLEAR";
    case port_link_sm_t::PORT_LINK_SM_UP:
        return "UP";
    default:
        return "INVALID";
    }
}

enum class port_event_t {
    PORT_EVENT_LINK_NONE = 0,
    PORT_EVENT_LINK_DOWN = 1,
    PORT_EVENT_LINK_UP   = 2
};

typedef enum cable_type_e {
    CABLE_TYPE_NONE,
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

enum class port_admin_state_t {
    PORT_ADMIN_STATE_NONE    = 0,
    PORT_ADMIN_STATE_DOWN    = 1,
    PORT_ADMIN_STATE_UP      = 2
};

enum class port_oper_status_t {
    PORT_OPER_STATUS_NONE = 0,
    PORT_OPER_STATUS_UP   = 1,
    PORT_OPER_STATUS_DOWN = 2
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

enum class port_loopback_mode_t {
    PORT_LOOPBACK_MODE_NONE,    // Disable loopback
    PORT_LOOPBACK_MODE_MAC,     // MAC loopback
    PORT_LOOPBACK_MODE_PHY,     // PHY/Serdes loopback
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

    // CU
    XCVR_PID_QSFP_100G_CR4,
    XCVR_PID_QSFP_40GBASE_CR4,
    XCVR_PID_SFP_25GBASE_CR_S,
    XCVR_PID_SFP_25GBASE_CR_L,
    XCVR_PID_SFP_25GBASE_CR_N,

    // Fiber
    XCVR_PID_QSFP_100G_AOC = 50,
    XCVR_PID_QSFP_100G_ACC,
    XCVR_PID_QSFP_100G_SR4,
    XCVR_PID_QSFP_100G_LR4,
    XCVR_PID_QSFP_100G_ER4,
    XCVR_PID_QSFP_40GBASE_ER4,
    XCVR_PID_QSFP_40GBASE_SR4,
    XCVR_PID_QSFP_40GBASE_LR4,
    XCVR_PID_QSFP_40GBASE_AOC,
    XCVR_PID_SFP_25GBASE_SR,
    XCVR_PID_SFP_25GBASE_LR,
    XCVR_PID_SFP_25GBASE_ER,
    XCVR_PID_SFP_25GBASE_AOC,
    XCVR_PID_SFP_10GBASE_SR,
    XCVR_PID_SFP_10GBASE_LR,
    XCVR_PID_SFP_10GBASE_LRM,
    XCVR_PID_SFP_10GBASE_ER,
    XCVR_PID_SFP_10GBASE_AOC,
    XCVR_PID_SFP_10GBASE_CU,
    XCVR_PID_QSFP_100G_CWDM4,
    XCVR_PID_QSFP_100G_PSM4,
    XCVR_PID_SFP_25GBASE_ACC,
} xcvr_pid_t;

typedef struct port_an_args_s {
    bool         fec_ability;
    uint32_t     user_cap;
    uint32_t     fec_request;
} port_an_args_t;

typedef struct xcvr_sprom_data_t {
    uint32_t  length_smf_km;
    uint32_t  length_smf;
    uint32_t  length_om1;
    uint32_t  length_om2;
    uint32_t  length_om3;
    uint32_t  length_dac;
    uint8_t   vendor_name[16];    // vendor name
    uint8_t   vendor_pn[16];      // vendor part number
    uint8_t   vendor_rev[4];      // vendor revision
    uint8_t   vendor_sn[16];      // serial number
} xcvr_sprom_data_t;

typedef struct xcvr_event_info_s {
    uint32_t          phy_port;       // transceiver port number
    uint32_t          port_num;       // front panel port number
    xcvr_state_t      state;
    xcvr_pid_t        pid;
    cable_type_t      cable_type;     // copper or fiber
    port_an_args_t    *port_an_args;  // auto-neg params based on cable
    uint8_t           xcvr_sprom[XCVR_SPROM_SIZE];
} xcvr_event_info_t;

typedef struct port_event_info_s {
    uint32_t           logical_port;    // logical port number
    port_oper_status_t oper_status;     // oper status of the port
    port_event_t       event;           // link UP/Down event
    port_speed_t       speed;           // speed of the port
    port_type_t        type;            // type of the port
    port_fec_type_t    fec_type;        // type of the port
    uint32_t           num_lanes;       // number of lanes for the port
    bool               auto_neg_enable; // operational state for AN
} port_event_info_t;

typedef void (*port_event_notify_t)(port_event_info_t *port_event_info);

typedef enum lif_state {
    LIF_STATE_NONE = 0,
    LIF_STATE_UP,
    LIF_STATE_DOWN,
} lif_state_t;

static inline
std::string xcvrStateToStr (xcvr_state_t state) {
    std::string str;

    switch (state) {
    case XCVR_REMOVED:
        str = "xcvr removed";
        break;
    case XCVR_INSERTED:
        str = "xcvr inserted";
        break;
    case XCVR_SPROM_PENDING:
        str = "xcvr SPROM pending";
        break;
    case XCVR_SPROM_READ:
        str = "xcvr SPROM read";
        break;
    case XCVR_SPROM_READ_ERR:
        str = "xcvr SPROM Read Err";
        break;
    default:
        str = "xcvr Invalid State";
    }

    return str;
}

// packet type classification
typedef enum pkt_type_e {
    PKT_TYPE_NONE,
    PKT_TYPE_ARP,
    PKT_TYPE_DHCP,
    PKT_TYPE_NDP,
    PKT_TYPE_IPV4,
    PKT_TYPE_IPV6,
} pkt_type_t;

static inline
std::string pkttype2str (pkt_type_t pkt_type) {
    switch (pkt_type) {
    case PKT_TYPE_NONE:
        return "unknown";
    case PKT_TYPE_ARP:
        return "ARP";
    case PKT_TYPE_DHCP:
        return "DHCP";
    case PKT_TYPE_NDP:
        return "NDP";
    case PKT_TYPE_IPV4:
        return "IPV4";
    case PKT_TYPE_IPV6:
        return "IPV6";
    }
    return "invalid pkt type";
}

}    // namespace types
}    // namespace sdk

using sdk::types::port_speed_t;
using sdk::types::port_type_t;
using sdk::types::port_admin_state_t;
using sdk::types::port_oper_status_t;
using sdk::types::port_fec_type_t;
using sdk::types::port_pause_type_t;
using sdk::types::port_event_t;
using sdk::types::port_breakout_mode_t;
using sdk::types::mem_addr_t;
using sdk::types::q_notify_mode_t;
using sdk::types::cable_type_t;
using sdk::types::xcvr_event_info_t;
using sdk::types::xcvr_sprom_data_t;
using sdk::types::port_event_notify_t;
using sdk::types::port_event_info_t;
using sdk::types::port_an_args_t;
using sdk::types::port_link_sm_t;
using sdk::types::port_loopback_mode_t;
using sdk::types::xcvr_type_t;
using sdk::types::xcvr_state_t;
using sdk::types::xcvr_pid_t;
using sdk::types::lif_state_t;
using sdk::types::pkt_type_t;
using sdk::types::monitor_type_t;

#endif    // __SDK_TYPES_HPP__

