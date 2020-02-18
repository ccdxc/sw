//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// server side implementation for Learn service
///
//----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/learn.hpp"

Status
LearnSvcImpl::LearnMACGet(ServerContext *context,
                          const ::pds::LearnMACGetRequest* proto_req,
                          pds::LearnMACGetResponse *proto_rsp) {
    proto_rsp->set_apistatus(
               types::ApiStatus::API_STATUS_OPERATION_NOT_ALLOWED);
    return Status::OK;
}

Status
LearnSvcImpl::LearnIPGet(ServerContext* context,
                         const ::pds::LearnIPGetRequest* proto_req,
                         pds::LearnIPGetResponse* proto_rsp) {
    proto_rsp->set_apistatus(
               types::ApiStatus::API_STATUS_OPERATION_NOT_ALLOWED);
    return Status::OK;
}

Status
LearnSvcImpl::LearnStatsGet(ServerContext* context,
                            const types::Empty* proto_req,
                            pds::LearnStatsGetResponse* proto_rsp) {
    proto_rsp->set_apistatus(
               types::ApiStatus::API_STATUS_OPERATION_NOT_ALLOWED);
    return Status::OK;
}
