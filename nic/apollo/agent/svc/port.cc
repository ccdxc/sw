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

Status
PortSvcImpl::PortGet(ServerContext *context,
                     const pds::PortGetRequest *proto_req,
                     pds::PortGetResponse *proto_rsp) {
    sdk_ret_t ret;

    PDS_TRACE_VERBOSE("Received Port Get");
    if (proto_req) {
        for (int i = 0; i < proto_req->id_size(); i ++) {
            ret = api::port_get(proto_req->id(i), pds_port_to_proto,
                                proto_rsp);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        }
        if (proto_req->id_size() == 0) {
            ret = api::port_get(0, pds_port_to_proto, proto_rsp);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
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
    pds_port_proto_to_port_args(&port_args, proto_req->spec());
    ret = api::update_port(proto_req->spec().id(), &port_args);
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));

    return Status::OK;
}

Status
PortSvcImpl::StartAacsServer(ServerContext *context,
                             const pds::AacsRequest *proto_req,
                             Empty *proto_rsp) {
    PDS_TRACE_VERBOSE("Received AACS Server Start");
    if (proto_req) {
        api::start_aacs_server(proto_req->aacsserverport());
    }
    return Status::OK;
}

Status
PortSvcImpl::StopAacsServer(ServerContext *context,
                            const Empty *proto_req,
                            Empty *proto_rsp) {
    PDS_TRACE_VERBOSE("Received AACS Server Stop");
    api::stop_aacs_server();
    return Status::OK;
}
