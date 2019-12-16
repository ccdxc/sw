//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// HAL Table service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal_trace.hpp"
#include "nic/hal/svc/table_svc.hpp"
#include "nic/hal/src/debug/table.hpp"

Status
TableServiceImpl::TableMetadataGet(ServerContext *context,
                                   const Empty *req,
                                   TableMetadataResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Received table metadata get");
    hal::table_metadata_get(rsp);
    return Status::OK;
}

Status
TableServiceImpl::TableGet(ServerContext *context,
                           const TableRequestMsg *req,
                           TableResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->spec_size();

    HAL_TRACE_DEBUG("Received table get");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }
    for (i = 0; i < nreqs; i++) {
        auto spec = req->spec(i);
        hal::table_get(spec, rsp);
    }
    return Status::OK;
}
