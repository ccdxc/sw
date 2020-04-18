//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/sdk/linkmgr/port_mac.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/apollo/api/port.hpp"
// TODO:
// @rsrikanth -> including this is a clear layer violation, we can only
//               include from api/include, not from api/ like this in agent code
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/svc/port_svc.hpp"

Status
PortSvcImpl::PortGet(ServerContext *context,
                     const pds::PortGetRequest *proto_req,
                     pds::PortGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_obj_key_t key;

    PDS_TRACE_VERBOSE("Received Port Get");
    if (proto_req) {
        for (int i = 0; i < proto_req->id_size(); i ++) {
            pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
            ret = api::port_get(&key, pds_port_to_proto, proto_rsp);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        }
        if (proto_req->id_size() == 0) {
            ret = api::port_get(&k_pds_obj_key_invalid,
                                pds_port_to_proto, proto_rsp);
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
    pds_obj_key_t key;
    port_args_t port_args;

    PDS_TRACE_VERBOSE("Received Port Update");
    pds_port_proto_to_port_args(&port_args, proto_req->spec());
    pds_obj_key_proto_to_api_spec(&key, proto_req->spec().id());
    ret = api::update_port(&key, &port_args);
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));

    return Status::OK;
}
