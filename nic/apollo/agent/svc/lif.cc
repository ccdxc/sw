//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/lif.hpp"
#include "nic/apollo/api/include/pds_lif.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/specs.hpp"

Status
IfSvcImpl::LifGet(ServerContext *context,
                  const pds::LifGetRequest *proto_req,
                  pds::LifGetResponse *proto_rsp) {
    sdk_ret_t ret;

    PDS_TRACE_VERBOSE("Received Lif Get");
    if (proto_req) {
        for (int i = 0; i < proto_req->lifid_size(); i ++) {
            pds_lif_spec_t spec = {0};
            pds_lif_key_t key = proto_req->lifid(i);
            ret = pds_lif_read(&key, &spec);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            pds_lif_api_spec_to_proto(&spec, proto_rsp);
        }
        if (proto_req->lifid_size() == 0) {
            ret = pds_lif_read_all(pds_lif_api_spec_to_proto, proto_rsp);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        }
    }
    return Status::OK;
}
