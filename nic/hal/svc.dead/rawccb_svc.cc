//------------------------------------------------------------------------------
// RAWCCB (L7 Raw Chain CB) service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/rawccb_svc.hpp"
#include "nic/hal/src/rawccb.hpp"

Status
RawcCbServiceImpl::RawcCbCreate(ServerContext *context,
                                const RawcCbRequestMsg *req,
                                RawcCbResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();
    RawcCbResponse    *response;

    HAL_TRACE_DEBUG("Rcvd RawcCb Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::rawccb_create(spec, response);
    }
    return Status::OK;
}

Status
RawcCbServiceImpl::RawcCbUpdate(ServerContext *context,
                                const RawcCbRequestMsg *req,
                                RawcCbResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();
    RawcCbResponse    *response;

    HAL_TRACE_DEBUG("Rcvd RawcCb Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::rawccb_update(spec, response);
    }
    return Status::OK;
}


Status
RawcCbServiceImpl::RawcCbDelete(ServerContext *context,
                                const RawcCbDeleteRequestMsg *req,
                                RawcCbDeleteResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();

    HAL_TRACE_DEBUG("Rcvd RawcCb Delete Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        auto spec = req->request(i);
        hal::rawccb_delete(spec, rsp);
    }
    return Status::OK;
}

Status
RawcCbServiceImpl::RawcCbGet(ServerContext *context,
                             const RawcCbGetRequestMsg *req,
                             RawcCbGetResponseMsg *rsp)
{
    uint32_t            i, nreqs = req->request_size();
    RawcCbGetResponse    *response;

    HAL_TRACE_DEBUG("Rcvd RawcCb Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::rawccb_get(request, response);
    }
    return Status::OK;
}
