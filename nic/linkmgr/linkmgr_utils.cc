// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "linkmgr_src.hpp"

namespace linkmgr {

port_speed_t
port_speed_spec_to_sdk_port_speed (::port::PortSpeed speed)
{
    switch (speed) {
    case ::port::PORT_SPEED_1G:
        return port_speed_t::PORT_SPEED_1G;

    case ::port::PORT_SPEED_10G:
        return port_speed_t::PORT_SPEED_10G;

    case ::port::PORT_SPEED_25G:
        return port_speed_t::PORT_SPEED_25G;

    case ::port::PORT_SPEED_40G:
        return port_speed_t::PORT_SPEED_40G;

    case ::port::PORT_SPEED_50G:
        return port_speed_t::PORT_SPEED_50G;

    case ::port::PORT_SPEED_100G:
        return port_speed_t::PORT_SPEED_100G;

    default:
        return port_speed_t::PORT_SPEED_NONE;
    }

    return port_speed_t::PORT_SPEED_NONE;
}

::port::PortSpeed
sdk_port_speed_to_port_speed_spec (port_speed_t speed)
{
    switch (speed) {
    case port_speed_t::PORT_SPEED_1G:
        return ::port::PORT_SPEED_1G;

    case port_speed_t::PORT_SPEED_10G:
        return ::port::PORT_SPEED_10G;

    case port_speed_t::PORT_SPEED_25G:
        return ::port::PORT_SPEED_25G;

    case port_speed_t::PORT_SPEED_40G:
        return ::port::PORT_SPEED_40G;

    case port_speed_t::PORT_SPEED_50G:
        return ::port::PORT_SPEED_50G;

    case port_speed_t::PORT_SPEED_100G:
        return ::port::PORT_SPEED_100G;

    default:
        return ::port::PORT_SPEED_NONE;
    }

    return ::port::PORT_SPEED_NONE;
}

port_type_t
port_type_spec_to_sdk_port_type (::port::PortType type)
{
    switch (type) {
    case ::port::PORT_TYPE_ETH:
        return port_type_t::PORT_TYPE_ETH;

    case ::port::PORT_TYPE_MGMT:
        return port_type_t::PORT_TYPE_MGMT;

    default:
        return port_type_t::PORT_TYPE_NONE;
    }

    return port_type_t::PORT_TYPE_NONE;
}

::port::PortType
sdk_port_type_to_port_type_spec (port_type_t type)
{
    switch (type) {
    case port_type_t::PORT_TYPE_ETH:
        return ::port::PORT_TYPE_ETH;

    case port_type_t::PORT_TYPE_MGMT:
        return ::port::PORT_TYPE_MGMT;

    default:
        return ::port::PORT_TYPE_NONE;
    }

    return ::port::PORT_TYPE_NONE;
}

::port::PortAdminState
sdk_port_admin_st_to_port_admin_st_spec (port_admin_state_t admin_st)
{
    switch(admin_st) {
    case port_admin_state_t::PORT_ADMIN_STATE_DOWN:
        return ::port::PORT_ADMIN_STATE_DOWN;

    case port_admin_state_t::PORT_ADMIN_STATE_UP:
        return ::port::PORT_ADMIN_STATE_UP;

    default:
        return ::port::PORT_ADMIN_STATE_NONE;
    }

    return ::port::PORT_ADMIN_STATE_NONE;
}

port_admin_state_t
port_admin_st_spec_to_sdk_port_admin_st (::port::PortAdminState admin_st)
{
    switch(admin_st) {
    case ::port::PORT_ADMIN_STATE_DOWN:
        return port_admin_state_t::PORT_ADMIN_STATE_DOWN;

    case ::port::PORT_ADMIN_STATE_UP:
        return port_admin_state_t::PORT_ADMIN_STATE_UP;

    default:
        return port_admin_state_t::PORT_ADMIN_STATE_NONE;
    }

    return port_admin_state_t::PORT_ADMIN_STATE_NONE;
}

::port::PortFecType
sdk_port_fec_type_to_port_fec_type_spec (port_fec_type_t fec_type)
{
    switch(fec_type) {
    case port_fec_type_t::PORT_FEC_TYPE_RS:
        return ::port::PORT_FEC_TYPE_RS;

    case port_fec_type_t::PORT_FEC_TYPE_FC:
        return ::port::PORT_FEC_TYPE_FC;

    default:
        return ::port::PORT_FEC_TYPE_NONE;
    }

    return ::port::PORT_FEC_TYPE_NONE;
}

port_pause_type_t
port_pause_type_spec_to_sdk_port_pause_type (::port::PortPauseType pause_type)
{
    switch(pause_type) {
    case ::port::PORT_PAUSE_TYPE_LINK:
        return port_pause_type_t::PORT_PAUSE_TYPE_LINK;

    case ::port::PORT_PAUSE_TYPE_PFC:
        return port_pause_type_t::PORT_PAUSE_TYPE_PFC;

    default:
        return port_pause_type_t::PORT_PAUSE_TYPE_NONE;
    }

    return port_pause_type_t::PORT_PAUSE_TYPE_NONE;
}

port_fec_type_t
port_fec_type_spec_to_sdk_port_fec_type (::port::PortFecType fec_type)
{
    switch(fec_type) {
    case ::port::PORT_FEC_TYPE_RS:
        return port_fec_type_t::PORT_FEC_TYPE_RS;

    case ::port::PORT_FEC_TYPE_FC:
        return port_fec_type_t::PORT_FEC_TYPE_FC;

    default:
        return port_fec_type_t::PORT_FEC_TYPE_NONE;
    }

    return port_fec_type_t::PORT_FEC_TYPE_NONE;
}

::port::PortOperStatus
sdk_port_oper_st_to_port_oper_st_spec (port_oper_status_t oper_st)
{
    switch (oper_st) {
    case port_oper_status_t::PORT_OPER_STATUS_UP:
        return ::port::PORT_OPER_STATUS_UP;

    case port_oper_status_t::PORT_OPER_STATUS_DOWN:
        return ::port::PORT_OPER_STATUS_DOWN;

    default:
        return ::port::PORT_OPER_STATUS_NONE;
    }
    return ::port::PORT_OPER_STATUS_NONE;
}

::port::PortBreakoutMode
sdk_port_breakout_mode_to_port_breakout_mode_spec (
                            port_breakout_mode_t breakout_mode)
{
    switch(breakout_mode) {
    case port_breakout_mode_t::BREAKOUT_MODE_4x25G:
        return ::port::PORT_BREAKOUT_MODE_4x25G;

    case port_breakout_mode_t::BREAKOUT_MODE_4x10G:
        return ::port::PORT_BREAKOUT_MODE_4x10G;

    case port_breakout_mode_t::BREAKOUT_MODE_2x50G:
        return ::port::PORT_BREAKOUT_MODE_2x50G;

    default:
        return ::port::PORT_BREAKOUT_MODE_NONE;
    }

    return ::port::PORT_BREAKOUT_MODE_NONE;
}

} // namespace linkmgr
