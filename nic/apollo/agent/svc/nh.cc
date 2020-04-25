//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/nh_svc.hpp"

Status
NhSvcImpl::NexthopCreate(ServerContext *context,
                         const pds::NexthopRequest *proto_req,
                         pds::NexthopResponse *proto_rsp) {
    pds_svc_nexthop_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
NhSvcImpl::NexthopUpdate(ServerContext *context,
                         const pds::NexthopRequest *proto_req,
                         pds::NexthopResponse *proto_rsp) {
    pds_svc_nexthop_update(proto_req, proto_rsp);
    return Status::OK;
}

Status
NhSvcImpl::NexthopDelete(ServerContext *context,
                         const pds::NexthopDeleteRequest *proto_req,
                         pds::NexthopDeleteResponse *proto_rsp) {
    pds_svc_nexthop_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
NhSvcImpl::NexthopGet(ServerContext *context,
                      const pds::NexthopGetRequest *proto_req,
                      pds::NexthopGetResponse *proto_rsp) {
    pds_svc_nexthop_get(proto_req, proto_rsp);
    return Status::OK;
}

Status
NhSvcImpl::NhGroupCreate(ServerContext *context,
                         const pds::NhGroupRequest *proto_req,
                         pds::NhGroupResponse *proto_rsp) {
    pds_svc_nhgroup_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
NhSvcImpl::NhGroupUpdate(ServerContext *context,
                         const pds::NhGroupRequest *proto_req,
                         pds::NhGroupResponse *proto_rsp) {
    pds_svc_nhgroup_update(proto_req, proto_rsp);
    return Status::OK;
}

Status
NhSvcImpl::NhGroupDelete(ServerContext *context,
                         const pds::NhGroupDeleteRequest *proto_req,
                         pds::NhGroupDeleteResponse *proto_rsp) {
    pds_svc_nhgroup_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
NhSvcImpl::NhGroupGet(ServerContext *context,
                      const pds::NhGroupGetRequest *proto_req,
                      pds::NhGroupGetResponse *proto_rsp) {
    pds_svc_nhgroup_get(proto_req, proto_rsp);
    return Status::OK;
}
