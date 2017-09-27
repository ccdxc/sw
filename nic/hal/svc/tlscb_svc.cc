//------------------------------------------------------------------------------
// TLSCB service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/tlscb_svc.hpp"
#include "nic/hal/src/tlscb.hpp"

Status
TlsCbServiceImpl::TlsCbCreate(ServerContext *context,
                              const TlsCbRequestMsg *req,
                              TlsCbResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();
    TlsCbResponse    *response;

    HAL_TRACE_DEBUG("Rcvd TlsCb Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::tlscb_create(spec, response);
    }
    return Status::OK;
}

Status
TlsCbServiceImpl::TlsCbUpdate(ServerContext *context,
                              const TlsCbRequestMsg *req,
                              TlsCbResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();
    TlsCbResponse    *response;

    HAL_TRACE_DEBUG("Rcvd TlsCb Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::tlscb_update(spec, response);
    }
    return Status::OK;
}


Status
TlsCbServiceImpl::TlsCbDelete(ServerContext *context,
                              const TlsCbDeleteRequestMsg *req,
                              TlsCbDeleteResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();

    HAL_TRACE_DEBUG("Rcvd TlsCb Delete Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        auto spec = req->request(i);
        hal::tlscb_delete(spec, rsp);
    }
    return Status::OK;
}

Status
TlsCbServiceImpl::TlsCbGet(ServerContext *context,
                           const TlsCbGetRequestMsg *req,
                           TlsCbGetResponseMsg *rsp)
{
    uint32_t            i, nreqs = req->request_size();
    TlsCbGetResponse    *response;

    HAL_TRACE_DEBUG("Rcvd TlsCb Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::tlscb_get(request, response);
    }
    return Status::OK;
}
