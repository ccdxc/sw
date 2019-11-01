//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/metaswitch/svc/specs.hpp"
#include "nic/metaswitch/include/bgp_api.hpp"
#include "bgp.hpp"
#include "specs.hpp"

Status
BGPSvcImpl::BGPGlobalSpecCreate(ServerContext *context,
                      const pds::BGPGlobalSpec *proto_req,
                      pds::BGPGlobalSpecResponseMsg *proto_rsp) {
    //bgp_global_spec_t* api_spec;

    if ((proto_req == NULL)) { // || (proto_req->request_size() == 0)) {
     //   proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::CANCELLED;
    }
#if 0
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec =
            (bgp_global_spec_t*)core::agent_state::state()->vpc_slab()->alloc();
        if (api_spec == NULL) {
            ret = SDK_RET_OOM;
            goto end;
        }
        auto proto_spec = proto_req->request(i);
        pds_bgp_global_proto_to_api_spec(api_spec, proto_spec);
//        ret = core::vpc_create(&key, api_spec, bctxt);
//        if (ret != SDK_RET_OK) {
//            goto end;
//        }
    }

    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
#endif
    return Status::OK;

//end:

    //proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
 //   return Status::CANCELLED;
}

    Status BGPSvcImpl::BGPGlobalSpecUpdate(ServerContext *context, const pds::BGPGlobalSpec *req,
                     pds::BGPGlobalSpecResponseMsg *rsp) {
return Status::OK;
}
    Status BGPSvcImpl::BGPGlobalSpecDelete(ServerContext *context,
                     const pds::BGPGlobalSpec *proto_req,
                     pds::BGPGlobalSpecResponseMsg *proto_rsp) {
return Status::OK;
}
    Status BGPSvcImpl::BGPGlobalSpecGet(ServerContext *context,
                  const pds::BGPGlobalSpec *proto_req,
                  pds::BGPGlobalSpecResponseMsg *proto_rsp) {
return Status::OK;
}
