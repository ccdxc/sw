//------------------------------------------------------------------------------
// RAWRCB (L7 Raw Redirect CB) service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/rawrcb_svc.hpp"
#include "nic/hal/src/rawrcb.hpp"

Status
RawrCbServiceImpl::RawrCbCreate(ServerContext *context,
                                const RawrCbRequestMsg *req,
                                RawrCbResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();
    RawrCbResponse    *response;

    HAL_TRACE_DEBUG("Rcvd RawrCb Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::rawrcb_create(spec, response);
    }
    return Status::OK;
}

Status
RawrCbServiceImpl::RawrCbUpdate(ServerContext *context,
                                const RawrCbRequestMsg *req,
                                RawrCbResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();
    RawrCbResponse    *response;

    HAL_TRACE_DEBUG("Rcvd RawrCb Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::rawrcb_update(spec, response);
    }
    return Status::OK;
}


Status
RawrCbServiceImpl::RawrCbDelete(ServerContext *context,
                                const RawrCbDeleteRequestMsg *req,
                                RawrCbDeleteResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();

    HAL_TRACE_DEBUG("Rcvd RawrCb Delete Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        auto spec = req->request(i);
        hal::rawrcb_delete(spec, rsp);
    }
    return Status::OK;
}

Status
RawrCbServiceImpl::RawrCbGet(ServerContext *context,
                             const RawrCbGetRequestMsg *req,
                             RawrCbGetResponseMsg *rsp)
{
    uint32_t            i, nreqs = req->request_size();
    RawrCbGetResponse    *response;

    HAL_TRACE_DEBUG("Rcvd RawrCb Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::rawrcb_get(request, response);
    }
    return Status::OK;
}
