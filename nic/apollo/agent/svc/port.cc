//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/sdk/linkmgr/port_mac.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/apollo/agent/svc/port.hpp"
#include "nic/apollo/api/port.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/specs.hpp"

static inline void
pds_port_stats_fill (pds::PortStats *stats,
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
}

static inline void
pds_port_specs_fill (pds::PortSpec *spec, sdk::linkmgr::port_args_t *port_info)
{
    spec->set_id(port_info->port_num);
    switch(port_info->admin_state) {
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
}

static inline void
pds_port_status_fill (pds::PortStatus *status,
                      sdk::linkmgr::port_args_t *port_info)
{
    auto link_status = status->mutable_linkstatus();
    auto xcvr_status = status->mutable_xcvrstatus();

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
        link_status->set_portspeed(pds::PORT_SPEED_1G);
        break;
    case port_speed_t::PORT_SPEED_10G:
        link_status->set_portspeed(pds::PORT_SPEED_10G);
        break;
    case port_speed_t::PORT_SPEED_25G:
        link_status->set_portspeed(pds::PORT_SPEED_25G);
        break;
    case port_speed_t::PORT_SPEED_40G:
        link_status->set_portspeed(pds::PORT_SPEED_40G);
        break;
    case port_speed_t::PORT_SPEED_50G:
        link_status->set_portspeed(pds::PORT_SPEED_50G);
        break;
    case port_speed_t::PORT_SPEED_100G:
        link_status->set_portspeed(pds::PORT_SPEED_100G);
        break;
    default:
        link_status->set_portspeed(pds::PORT_SPEED_NONE);
        break;
    }

    xcvr_status->set_port(port_info->xcvr_event_info.phy_port);
    xcvr_status->set_state(pds::PortXcvrState(port_info->xcvr_event_info.state));
    xcvr_status->set_pid(pds::PortXcvrPid(port_info->xcvr_event_info.pid));
    xcvr_status->set_mediatype(pds::MediaType(port_info->xcvr_event_info.cable_type));
    xcvr_status->set_xcvrsprom(std::string((char*)&port_info->xcvr_event_info.xcvr_sprom));
}

void
pds_port_fill (sdk::linkmgr::port_args_t *port_info, void *ctxt)
{
    pds::PortGetResponse *rsp = (pds::PortGetResponse *)ctxt;
    pds::Port *port = rsp->add_response();
    pds::PortSpec *spec = port->mutable_spec();
    pds::PortStats *stats = port->mutable_stats();
    pds::PortStatus *status = port->mutable_status();

    pds_port_specs_fill(spec, port_info);
    pds_port_stats_fill(stats, port_info);
    pds_port_status_fill(status, port_info);
}

Status
PortSvcImpl::PortGet(ServerContext *context,
                     const pds::PortGetRequest *proto_req,
                     pds::PortGetResponse *proto_rsp) {
    sdk_ret_t ret;

    PDS_TRACE_VERBOSE("Received Port Get");

    if (proto_req) {
        for (int i = 0; i < proto_req->id_size(); i ++) {
            if ((ret = api::port_get(proto_req->id(i), pds_port_fill,
                                     proto_rsp)) != SDK_RET_OK) {
                proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            }
        }
        if (proto_req->id_size() == 0) {
            if ((ret = api::port_get(0, pds_port_fill,
                                     proto_rsp)) != SDK_RET_OK) {
                proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            }
        }
    }
    return Status::OK;
}

Status
PortSvcImpl::PortUpdate(ServerContext *context,
                        const pds::PortUpdateRequest *proto_req,
                        pds::PortUpdateResponse *proto_rsp) {
    sdk_ret_t ret;
    port_args_t port_args;

    PDS_TRACE_VERBOSE("Received Port Update");
    proto_port_spec_to_port_args(&port_args, proto_req->spec());
    ret = api::update_port(proto_req->spec().id(), &port_args);
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));

    return Status::OK;
}
