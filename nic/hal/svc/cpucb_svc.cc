//------------------------------------------------------------------------------
// CPU Cb service implementation
//------------------------------------------------------------------------------

#include <base.h>
#include <trace.hpp>
#include <cpucb_svc.hpp>
#include <cpucb.hpp>

Status
CpuCbServiceImpl::CpuCbCreate(ServerContext *context,
                              const CpuCbRequestMsg *req,
                              CpuCbResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();
    CpuCbResponse    *response;

    HAL_TRACE_DEBUG("Rcvd CpuCb Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::cpucb_create(spec, response);
    }
    return Status::OK;
}

Status
CpuCbServiceImpl::CpuCbUpdate(ServerContext *context,
                              const CpuCbRequestMsg *req,
                              CpuCbResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();
    CpuCbResponse    *response;

    HAL_TRACE_DEBUG("Rcvd CpuCb Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::cpucb_update(spec, response);
    }
    return Status::OK;
}


Status
CpuCbServiceImpl::CpuCbDelete(ServerContext *context,
                              const CpuCbDeleteRequestMsg *req,
                              CpuCbDeleteResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();

    HAL_TRACE_DEBUG("Rcvd CpuCb Delete Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        auto spec = req->request(i);
        hal::cpucb_delete(spec, rsp);
    }
    return Status::OK;
}

Status
CpuCbServiceImpl::CpuCbGet(ServerContext *context,
                           const CpuCbGetRequestMsg *req,
                           CpuCbGetResponseMsg *rsp)
{
    uint32_t            i, nreqs = req->request_size();
    CpuCbGetResponse    *response;

    HAL_TRACE_DEBUG("Rcvd CpuCb Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::cpucb_get(request, response);
    }
    return Status::OK;
}
