//------------------------------------------------------------------------------
// WRing service implementation
//------------------------------------------------------------------------------

#include <base.h>
#include <trace.hpp>
#include <wring_svc.hpp>
#include <wring.hpp>

Status
WRingServiceImpl::WRingCreate(ServerContext *context,
                              const WRingRequestMsg *req,
                              WRingResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();
    WRingResponse    *response;

    HAL_TRACE_DEBUG("Rcvd WRing Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::wring_create(spec, response);
    }
    return Status::OK;
}

Status
WRingServiceImpl::WRingUpdate(ServerContext *context,
                              const WRingRequestMsg *req,
                              WRingResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();
    WRingResponse    *response;

    HAL_TRACE_DEBUG("Rcvd WRing Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::wring_update(spec, response);
    }
    return Status::OK;
}


Status
WRingServiceImpl::WRingDelete(ServerContext *context,
                              const WRingDeleteRequestMsg *req,
                              WRingDeleteResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd WRing Delete Request");
    return Status::OK;
}

Status
WRingServiceImpl::WRingGet(ServerContext *context,
                           const WRingGetRequestMsg *req,
                           WRingGetResponseMsg *rsp)
{
    uint32_t            i, nreqs = req->request_size();
    WRingGetResponse    *response;

    HAL_TRACE_DEBUG("Rcvd WRing Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::wring_get(request, response);
    }
    return Status::OK;
}
