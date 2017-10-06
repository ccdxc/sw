//------------------------------------------------------------------------------
// endpoint service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/endpoint_svc.hpp"
#include "nic/hal/src/endpoint.hpp"

Status
EndpointServiceImpl::EndpointCreate(ServerContext *context,
                                    const EndpointRequestMsg *req,
                                    EndpointResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    EndpointResponse     *response;

    HAL_TRACE_DEBUG("Rcvd Endpoint Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::endpoint_create(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
EndpointServiceImpl::EndpointUpdate(ServerContext *context,
                                    const EndpointUpdateRequestMsg *req,
                                    EndpointUpdateResponseMsg *rsp)
{
    uint32_t                i, nreqs = req->request_size();
    EndpointResponse        *response;

    HAL_TRACE_DEBUG("Rcvd Endpoint Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::endpoint_update(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
EndpointServiceImpl::EndpointDelete(ServerContext *context,
                                    const EndpointDeleteRequestMsg *req,
                                    EndpointDeleteResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Endpoint Delete Request");
    return Status::OK;
}

Status
EndpointServiceImpl::EndpointGet(ServerContext *context,
                                 const EndpointGetRequestMsg *req,
                                 EndpointGetResponseMsg *rsp)
{
    uint32_t                i, nreqs = req->request_size();

    HAL_TRACE_DEBUG("Rcvd Endpoint Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        hal::endpoint_get(request, rsp);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
