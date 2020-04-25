//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/service_svc.hpp"

Status
SvcImpl::SvcMappingCreate(ServerContext *context,
                          const pds::SvcMappingRequest *proto_req,
                          pds::SvcMappingResponse *proto_rsp) {
    pds_svc_service_mapping_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
SvcImpl::SvcMappingUpdate(ServerContext *context,
                          const pds::SvcMappingRequest *proto_req,
                          pds::SvcMappingResponse *proto_rsp) {
    pds_svc_service_mapping_update(proto_req, proto_rsp);
    return Status::OK;
}

Status
SvcImpl::SvcMappingDelete(ServerContext *context,
                          const pds::SvcMappingDeleteRequest *proto_req,
                          pds::SvcMappingDeleteResponse *proto_rsp) {
    pds_svc_service_mapping_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
SvcImpl::SvcMappingGet(ServerContext *context,
                       const pds::SvcMappingGetRequest *proto_req,
                       pds::SvcMappingGetResponse *proto_rsp) {
    pds_svc_service_mapping_get(proto_req, proto_rsp);
    return Status::OK;
}
