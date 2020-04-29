//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for port object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_PORT_SVC_HPP__
#define __AGENT_SVC_PORT_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/port.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/sdk/linkmgr/port_mac.hpp"
#include "nic/sdk/platform/drivers/xcvr.hpp"

static inline pds::PortLinkSM
pds_fsmstate_to_proto (sdk::types::port_link_sm_t fsm_state)
{
    switch (fsm_state) {
    case port_link_sm_t::PORT_LINK_SM_DISABLED:
        return pds::PORT_LINK_FSM_DISABLED;
    case port_link_sm_t::PORT_LINK_SM_ENABLED:
        return pds::PORT_LINK_FSM_ENABLED;
    case port_link_sm_t::PORT_LINK_SM_AN_CFG:
        return pds::PORT_LINK_FSM_AN_CFG;
    case port_link_sm_t::PORT_LINK_SM_AN_DISABLED:
        return pds::PORT_LINK_FSM_AN_DISABLED;
    case port_link_sm_t::PORT_LINK_SM_AN_START:
        return pds::PORT_LINK_FSM_AN_START;
    case port_link_sm_t::PORT_LINK_SM_AN_WAIT_HCD:
        return pds::PORT_LINK_FSM_AN_WAIT_HCD;
    case port_link_sm_t::PORT_LINK_SM_AN_COMPLETE:
        return pds::PORT_LINK_FSM_AN_COMPLETE;
    case port_link_sm_t::PORT_LINK_SM_SERDES_CFG:
        return pds::PORT_LINK_FSM_SERDES_CFG;
    case port_link_sm_t::PORT_LINK_SM_WAIT_SERDES_RDY:
        return pds::PORT_LINK_FSM_WAIT_SERDES_RDY;
    case port_link_sm_t::PORT_LINK_SM_MAC_CFG:
        return pds::PORT_LINK_FSM_MAC_CFG;
    case port_link_sm_t::PORT_LINK_SM_SIGNAL_DETECT:
        return pds::PORT_LINK_FSM_SIGNAL_DETECT;
    case port_link_sm_t::PORT_LINK_SM_AN_DFE_TUNING:
        return pds::PORT_LINK_FSM_AN_DFE_TUNING;
    case port_link_sm_t::PORT_LINK_SM_DFE_TUNING:
        return pds::PORT_LINK_FSM_DFE_TUNING;
    case port_link_sm_t::PORT_LINK_SM_DFE_DISABLED:
        return pds::PORT_LINK_FSM_DFE_DISABLED;
    case port_link_sm_t::PORT_LINK_SM_DFE_START_ICAL:
        return pds::PORT_LINK_FSM_DFE_START_ICAL;
    case port_link_sm_t::PORT_LINK_SM_DFE_WAIT_ICAL:
        return pds::PORT_LINK_FSM_DFE_WAIT_ICAL;
    case port_link_sm_t::PORT_LINK_SM_DFE_START_PCAL:
        return pds::PORT_LINK_FSM_DFE_START_PCAL;
    case port_link_sm_t::PORT_LINK_SM_DFE_WAIT_PCAL:
        return pds::PORT_LINK_FSM_DFE_WAIT_PCAL;
    case port_link_sm_t::PORT_LINK_SM_DFE_PCAL_CONTINUOUS:
        return pds::PORT_LINK_FSM_DFE_PCAL_CONTINUOUS;
    case port_link_sm_t::PORT_LINK_SM_CLEAR_MAC_REMOTE_FAULTS:
        return pds::PORT_LINK_FSM_CLEAR_MAC_REMOTE_FAULTS;
    case port_link_sm_t::PORT_LINK_SM_WAIT_MAC_SYNC:
        return pds::PORT_LINK_FSM_WAIT_MAC_SYNC;
    case port_link_sm_t::PORT_LINK_SM_WAIT_MAC_FAULTS_CLEAR:
        return pds::PORT_LINK_FSM_WAIT_MAC_FAULTS_CLEAR;
    case port_link_sm_t::PORT_LINK_SM_UP:
        return pds::PORT_LINK_FSM_UP;
    default:
        return pds::PORT_LINK_FSM_DISABLED;
    }
}

static inline port_fec_type_t
pds_port_proto_fec_type_to_sdk_fec_type (pds::PortFecType proto_fec_type)
{
    switch (proto_fec_type) {
    case pds::PORT_FEC_TYPE_NONE:
        return port_fec_type_t::PORT_FEC_TYPE_NONE;
    case pds::PORT_FEC_TYPE_FC:
        return port_fec_type_t::PORT_FEC_TYPE_FC;
    case pds::PORT_FEC_TYPE_RS:
        return port_fec_type_t::PORT_FEC_TYPE_RS;
    default:
        return port_fec_type_t::PORT_FEC_TYPE_NONE;
    }
}

static inline pds::PortFecType
pds_port_sdk_fec_type_to_proto_fec_type (port_fec_type_t sdk_fec_type)
{
    switch (sdk_fec_type) {
    case port_fec_type_t::PORT_FEC_TYPE_FC:
        return pds::PORT_FEC_TYPE_FC;
    case port_fec_type_t::PORT_FEC_TYPE_RS:
        return pds::PORT_FEC_TYPE_RS;
    default:
        return pds::PORT_FEC_TYPE_NONE;
    }
}

static inline port_speed_t
pds_port_proto_speed_to_sdk_speed (types::PortSpeed proto_port_speed)
{
    switch (proto_port_speed) {
    case types::PORT_SPEED_NONE:
        return port_speed_t::PORT_SPEED_NONE;
    case types::PORT_SPEED_1G:
        return port_speed_t::PORT_SPEED_1G;
    case types::PORT_SPEED_10G:
        return port_speed_t::PORT_SPEED_10G;
    case types::PORT_SPEED_25G:
        return port_speed_t::PORT_SPEED_25G;
    case types::PORT_SPEED_40G:
        return port_speed_t::PORT_SPEED_40G;
    case types::PORT_SPEED_50G:
        return port_speed_t::PORT_SPEED_50G;
    case types::PORT_SPEED_100G:
        return port_speed_t::PORT_SPEED_100G;
    default:
        return port_speed_t::PORT_SPEED_NONE;
    }
}

static inline types::PortSpeed
pds_port_sdk_speed_to_proto_speed (port_speed_t sdk_port_speed)
{
    switch (sdk_port_speed) {
    case port_speed_t::PORT_SPEED_1G:
        return types::PORT_SPEED_1G;
    case port_speed_t::PORT_SPEED_10G:
        return types::PORT_SPEED_10G;
    case port_speed_t::PORT_SPEED_25G:
        return types::PORT_SPEED_25G;
    case port_speed_t::PORT_SPEED_40G:
        return types::PORT_SPEED_40G;
    case port_speed_t::PORT_SPEED_50G:
        return types::PORT_SPEED_50G;
    case port_speed_t::PORT_SPEED_100G:
        return types::PORT_SPEED_100G;
    default:
        return types::PORT_SPEED_NONE;
    }
}

static inline port_admin_state_t
pds_port_proto_admin_state_to_sdk_admin_state (
                            pds::PortAdminState proto_admin_state)
{
    switch (proto_admin_state) {
    case pds::PORT_ADMIN_STATE_NONE:
        return port_admin_state_t::PORT_ADMIN_STATE_NONE;
    case pds::PORT_ADMIN_STATE_DOWN:
        return port_admin_state_t::PORT_ADMIN_STATE_DOWN;
    case pds::PORT_ADMIN_STATE_UP:
        return port_admin_state_t::PORT_ADMIN_STATE_UP;
    default:
        return port_admin_state_t::PORT_ADMIN_STATE_NONE;
    }
}

static inline pds::PortAdminState
pds_port_sdk_admin_state_to_proto_admin_state (port_admin_state_t sdk_admin_state)
{
    switch (sdk_admin_state) {
    case port_admin_state_t::PORT_ADMIN_STATE_DOWN:
        return pds::PORT_ADMIN_STATE_DOWN;
    case port_admin_state_t::PORT_ADMIN_STATE_UP:
        return pds::PORT_ADMIN_STATE_UP;
    default:
        return pds::PORT_ADMIN_STATE_NONE;
    }
}

static inline port_pause_type_t
pds_port_proto_pause_type_to_sdk_pause_type (pds::PortPauseType proto_pause_type)
{
    switch(proto_pause_type) {
    case pds::PORT_PAUSE_TYPE_LINK:
        return port_pause_type_t::PORT_PAUSE_TYPE_LINK;
    case pds::PORT_PAUSE_TYPE_PFC:
        return port_pause_type_t::PORT_PAUSE_TYPE_PFC;
    default:
        return port_pause_type_t::PORT_PAUSE_TYPE_NONE;
    }
}

static inline pds::PortPauseType
pds_port_sdk_pause_type_to_proto_pause_type (port_pause_type_t sdk_pause_type)
{
    switch (sdk_pause_type) {
    case port_pause_type_t::PORT_PAUSE_TYPE_LINK:
        return pds::PORT_PAUSE_TYPE_LINK;
    case port_pause_type_t::PORT_PAUSE_TYPE_PFC:
        return pds::PORT_PAUSE_TYPE_PFC;
    default:
        return pds::PORT_PAUSE_TYPE_NONE;
    }
}

static inline port_loopback_mode_t
pds_port_proto_loopback_mode_to_sdk_loopback_mode (
                        pds::PortLoopBackMode proto_loopback_mode)
{
    switch(proto_loopback_mode) {
    case pds::PORT_LOOPBACK_MODE_MAC:
        return port_loopback_mode_t::PORT_LOOPBACK_MODE_MAC;
    case pds::PORT_LOOPBACK_MODE_PHY:
        return port_loopback_mode_t::PORT_LOOPBACK_MODE_PHY;
    default:
        return port_loopback_mode_t::PORT_LOOPBACK_MODE_NONE;
    }
}

static inline pds::PortLoopBackMode
pds_port_sdk_loopback_mode_to_proto_loopback_mode (
                        port_loopback_mode_t sdk_loopback_mode)
{
    switch (sdk_loopback_mode) {
    case port_loopback_mode_t::PORT_LOOPBACK_MODE_MAC:
        return pds::PORT_LOOPBACK_MODE_MAC;
    case port_loopback_mode_t::PORT_LOOPBACK_MODE_PHY:
        return pds::PORT_LOOPBACK_MODE_PHY;
    default:
        return pds::PORT_LOOPBACK_MODE_NONE;
    }
}

static inline void
pds_port_proto_to_port_args (port_args_t *port_args,
                             const pds::PortSpec &spec)
{
    memset(port_args, 0, sizeof(port_args_t));

    switch (spec.type()) {
    case pds::PORT_TYPE_ETH:
        port_args->port_type = port_type_t::PORT_TYPE_ETH;
        break;
    case pds::PORT_TYPE_ETH_MGMT:
        port_args->port_type = port_type_t::PORT_TYPE_MGMT;
        break;
    default:
        port_args->port_type = port_type_t::PORT_TYPE_NONE;
        break;
    }
    port_args->admin_state =
               pds_port_proto_admin_state_to_sdk_admin_state(spec.adminstate());
    port_args->port_speed = pds_port_proto_speed_to_sdk_speed(spec.speed());
    port_args->fec_type =
                   pds_port_proto_fec_type_to_sdk_fec_type(spec.fectype());
    port_args->auto_neg_enable = spec.autonegen();
    port_args->debounce_time = spec.debouncetimeout();
    port_args->mtu = spec.mtu();
    port_args->pause =
                pds_port_proto_pause_type_to_sdk_pause_type(spec.pausetype());
    port_args->loopback_mode =
       pds_port_proto_loopback_mode_to_sdk_loopback_mode(spec.loopbackmode());
    port_args->num_lanes = spec.numlanes();

    // invoke after populating port_args from spec
    sdk::linkmgr::port_store_user_config(port_args);
}

static inline void
pds_port_stats_to_proto (pds::PortStats *stats,
                         sdk::linkmgr::port_args_t *port_info)
{
    if (port_info->port_type == port_type_t::PORT_TYPE_ETH) {
        for (uint32_t i = 0; i < MAX_MAC_STATS; i++) {
            auto macstats = stats->add_macstats();
            macstats->set_type(pds::MacStatsType(i));
            macstats->set_count(port_info->stats_data[i]);
        }
    } else if (port_info->port_type == port_type_t::PORT_TYPE_MGMT) {
        for (uint32_t i = 0; i < MAX_MGMT_MAC_STATS; i++) {
            auto macstats = stats->add_mgmtmacstats();
            macstats->set_type(pds::MgmtMacStatsType(i));
            macstats->set_count(port_info->stats_data[i]);
        }
    }
    stats->set_numlinkdown(port_info->num_link_down);
}

static inline void
pds_port_spec_to_proto (pds::PortSpec *spec,
                        const sdk::linkmgr::port_args_t *port_info)
{
    // @akoradha please fix this
    spec->set_id(port_info->port_an_args, PDS_MAX_KEY_LEN);
    spec->set_portnumber(ETH_IFINDEX_TO_PARENT_PORT(port_info->port_num));
    switch (port_info->port_type) {
    case port_type_t::PORT_TYPE_ETH:
        spec->set_type(pds::PORT_TYPE_ETH);
        break;
    case port_type_t::PORT_TYPE_MGMT:
        spec->set_type(pds::PORT_TYPE_ETH_MGMT);
        break;
    default:
        spec->set_type(pds::PORT_TYPE_NONE);
        break;
    }
    switch(port_info->user_admin_state) {
    case port_admin_state_t::PORT_ADMIN_STATE_DOWN:
        spec->set_adminstate(pds::PORT_ADMIN_STATE_DOWN);
        break;
    case port_admin_state_t::PORT_ADMIN_STATE_UP:
        spec->set_adminstate(pds::PORT_ADMIN_STATE_UP);
        break;
    default:
        spec->set_adminstate(pds::PORT_ADMIN_STATE_NONE);
        break;
    }
    spec->set_speed(pds_port_sdk_speed_to_proto_speed(
                                      port_info->port_speed));
    spec->set_fectype(pds_port_sdk_fec_type_to_proto_fec_type
                                      (port_info->user_fec_type));
    spec->set_autonegen(port_info->auto_neg_cfg);
    spec->set_debouncetimeout(port_info->debounce_time);
    spec->set_mtu(port_info->mtu);
    spec->set_pausetype(pds_port_sdk_pause_type_to_proto_pause_type
                                      (port_info->pause));
    spec->set_txpauseen(port_info->tx_pause_enable);
    spec->set_rxpauseen(port_info->rx_pause_enable);
    spec->set_loopbackmode(pds_port_sdk_loopback_mode_to_proto_loopback_mode(
                                     port_info->loopback_mode));
    spec->set_numlanes(port_info->num_lanes_cfg);
}

static inline void
pds_port_status_to_proto (pds::PortStatus *status,
                          const sdk::linkmgr::port_args_t *port_info)
{
    auto link_status = status->mutable_linkstatus();

    status->set_ifindex(port_info->port_num);
    status->set_fsmstate(pds_fsmstate_to_proto(port_info->link_sm));
    status->set_macid(port_info->mac_id);
    status->set_macch(port_info->mac_ch);
    switch (port_info->oper_status) {
    case port_oper_status_t::PORT_OPER_STATUS_UP:
        link_status->set_operstate(pds::PORT_OPER_STATUS_UP);
        break;
    case port_oper_status_t::PORT_OPER_STATUS_DOWN:
        link_status->set_operstate(pds::PORT_OPER_STATUS_DOWN);
        break;
    default:
        link_status->set_operstate(pds::PORT_OPER_STATUS_NONE);
        break;
    }

    switch (port_info->port_speed) {
    case port_speed_t::PORT_SPEED_1G:
        link_status->set_portspeed(types::PORT_SPEED_1G);
        break;
    case port_speed_t::PORT_SPEED_10G:
        link_status->set_portspeed(types::PORT_SPEED_10G);
        break;
    case port_speed_t::PORT_SPEED_25G:
        link_status->set_portspeed(types::PORT_SPEED_25G);
        break;
    case port_speed_t::PORT_SPEED_40G:
        link_status->set_portspeed(types::PORT_SPEED_40G);
        break;
    case port_speed_t::PORT_SPEED_50G:
        link_status->set_portspeed(types::PORT_SPEED_50G);
        break;
    case port_speed_t::PORT_SPEED_100G:
        link_status->set_portspeed(types::PORT_SPEED_100G);
        break;
    default:
        link_status->set_portspeed(types::PORT_SPEED_NONE);
        break;
    }
    link_status->set_autonegen(port_info->auto_neg_enable);
    link_status->set_numlanes(port_info->num_lanes);
    link_status->set_fectype(pds_port_sdk_fec_type_to_proto_fec_type
                                                      (port_info->fec_type));

    if (port_info->port_type != port_type_t::PORT_TYPE_MGMT) {
        const uint8_t *xcvr_info;
        auto xcvr_status = status->mutable_xcvrstatus();
        xcvr_info = port_info->xcvr_event_info.xcvr_sprom;
        xcvr_status->set_port(port_info->xcvr_event_info.phy_port);
        xcvr_status->set_state(pds::PortXcvrState(port_info->xcvr_event_info.state));
        xcvr_status->set_pid(pds::PortXcvrPid(port_info->xcvr_event_info.pid));
        xcvr_status->set_mediatype(pds::MediaType(port_info->xcvr_event_info.cable_type));
        xcvr_status->set_xcvrsprom(&port_info->xcvr_event_info.xcvr_sprom, XCVR_SPROM_SIZE);
        xcvr_status->set_vendorname(sdk::platform::xcvr_info_get_vendor_name(xcvr_info));
        xcvr_status->set_vendoroui(sdk::platform::xcvr_info_get_vendor_oui(xcvr_info));
        xcvr_status->set_serialnumber(sdk::platform::xcvr_info_get_vendor_serial_number(xcvr_info));
        xcvr_status->set_partnumber(sdk::platform::xcvr_info_get_vendor_part_number(xcvr_info));
        xcvr_status->set_revision(sdk::platform::xcvr_info_get_vendor_revision(port_info->xcvr_event_info.phy_port,
                                                                               xcvr_info));
    }
}

static inline void
pds_port_to_proto (sdk::linkmgr::port_args_t *port_info, void *ctxt)
{
    pds::PortGetResponse *rsp = (pds::PortGetResponse *)ctxt;
    pds::Port *port = rsp->add_response();
    pds::PortSpec *spec = port->mutable_spec();
    pds::PortStats *stats = port->mutable_stats();
    pds::PortStatus *status = port->mutable_status();

    pds_port_spec_to_proto(spec, port_info);
    pds_port_stats_to_proto(stats, port_info);
    pds_port_status_to_proto(status, port_info);
}

#endif    //__AGENT_SVC_PORT_SVC_HPP__
