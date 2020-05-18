//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/vnic.hpp"
#include "nic/apollo/agent/svc/vnic_svc.hpp"

Status
VnicSvcImpl::VnicCreate(ServerContext *context,
                        const pds::VnicRequest *proto_req,
                        pds::VnicResponse *proto_rsp) {
    pds_svc_vnic_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
VnicSvcImpl::VnicUpdate(ServerContext *context,
                        const pds::VnicRequest *proto_req,
                        pds::VnicResponse *proto_rsp) {
    pds_svc_vnic_update(proto_req, proto_rsp);
    return Status::OK;
}

Status
VnicSvcImpl::VnicDelete(ServerContext *context,
                        const pds::VnicDeleteRequest *proto_req,
                        pds::VnicDeleteResponse *proto_rsp) {
    pds_svc_vnic_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
VnicSvcImpl::VnicGet(ServerContext *context,
                     const pds::VnicGetRequest *proto_req,
                     pds::VnicGetResponse *proto_rsp) {
    pds_svc_vnic_get(proto_req, proto_rsp);
    return Status::OK;
}

Status VnicSvcImpl::VnicStatsReset(ServerContext *context, const types::Id *req,
                                   Empty *rsp) {
    pds_obj_key_t key;

    if (req->id().empty()) {
        pds_vnic_stats_reset(NULL);
    } else {
        pds_obj_key_proto_to_api_spec(&key, req->id());
        pds_vnic_stats_reset(&key);
    }
    return Status::OK;
}
