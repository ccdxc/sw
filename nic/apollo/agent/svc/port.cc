//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/port.hpp"
#include "nic/apollo/api/port.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/sdk/linkmgr/port_mac.hpp"
#include "nic/apollo/agent/svc/util.hpp"

void
pds_port_mac_stats_fill (sdk::linkmgr::port_args_t *port_info, void *ctxt) {
    pds::PortGetResponse *rsp = (pds::PortGetResponse *)ctxt;
    pds::Port *port = rsp->add_response();
    pds::PortSpec *spec = port->mutable_spec();
    pds::PortStats *stats = port->mutable_stats();

    spec->set_portid(port_info->port_num);
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

Status
PortSvcImpl::PortGet(ServerContext *context, const pds::PortGetRequest *proto_req,
                      pds::PortGetResponse *proto_rsp) {
    sdk_ret_t ret;

    PDS_TRACE_VERBOSE("Received Port Get");

    if (proto_req) {
        for (int i = 0; i < proto_req->portid_size(); i ++) {
            if ((ret = api::port_get(proto_req->portid(i), pds_port_mac_stats_fill, proto_rsp)) != SDK_RET_OK) {
                proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            }
        }
        if (proto_req->portid_size() == 0) {
            if ((ret = api::port_get(0, pds_port_mac_stats_fill, proto_rsp)) != SDK_RET_OK) {
                proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            }
        }
    }
    return Status::OK;
}
