//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/agent/core/core.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/svc/mapping_svc.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/hooks.hpp"
#include "nic/apollo/api/debug.hpp"
#include "nic/apollo/api/utils.hpp"
#include <malloc.h>

Status
MappingSvcImpl::MappingCreate(ServerContext *context,
                              const pds::MappingRequest *proto_req,
                              pds::MappingResponse *proto_rsp) {
    pds_svc_mapping_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
MappingSvcImpl::MappingUpdate(ServerContext *context,
                              const pds::MappingRequest *proto_req,
                              pds::MappingResponse *proto_rsp) {
    pds_svc_mapping_update(proto_req, proto_rsp);
    return Status::OK;
}

Status
MappingSvcImpl::MappingDelete(ServerContext *context,
                              const pds::MappingDeleteRequest *proto_req,
                              pds::MappingDeleteResponse *proto_rsp) {
    pds_svc_mapping_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
MappingSvcImpl::MappingGet(ServerContext *context,
                           const pds::MappingGetRequest *proto_req,
                           pds::MappingGetResponse *proto_rsp) {
    pds_svc_mapping_get(proto_req, proto_rsp);
    return Status::OK;
}
