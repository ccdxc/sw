//------------------------------------------------------------------------------
// WRing service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/wring_svc.hpp"
#include "nic/hal/src/wring.hpp"

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
WRingServiceImpl::WRingGetEntries(ServerContext *context,
                           const WRingGetEntriesRequestMsg *req,
                           WRingGetEntriesResponseMsg *rsp)
{
    uint32_t            i, nreqs = req->request_size();
    WRingGetEntriesResponse    *response;

    HAL_TRACE_DEBUG("Rcvd WRing Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::wring_get_entries(request, response);
    }
    return Status::OK;
}

Status
WRingServiceImpl::WRingGetMeta(ServerContext *context,
                           const WRingRequestMsg *req,
                           WRingGetMetaResponseMsg *rsp)
{
    uint32_t            i, nreqs = req->request_size();
    WRingGetMetaResponse    *response;

    HAL_TRACE_DEBUG("Rcvd WRing Get Meta Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::wring_get_meta(request, response);
    }
    return Status::OK;
}
