//------------------------------------------------------------------------------
// endpoint service implementation
//------------------------------------------------------------------------------

#include <base.h>
#include <trace.hpp>
#include <endpoint_svc.hpp>
#include <endpoint.hpp>

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

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::endpoint_create(spec, response);
    }
    return Status::OK;
}

Status
EndpointServiceImpl::EndpointUpdate(ServerContext *context,
                                    const EndpointRequestMsg *req,
                                    EndpointResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    EndpointResponse     *response;

    HAL_TRACE_DEBUG("Rcvd Endpoint Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::endpoint_update(spec, response);
    }
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

    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        hal::endpoint_get(request, rsp);
    }
    return Status::OK;
}
