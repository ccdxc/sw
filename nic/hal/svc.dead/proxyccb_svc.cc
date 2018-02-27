//------------------------------------------------------------------------------
// PROXYCCB (L7 Proxy Chain CB) service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/proxyccb_svc.hpp"
#include "nic/hal/src/proxyccb.hpp"

Status
ProxycCbServiceImpl::ProxycCbCreate(ServerContext *context,
                                    const ProxycCbRequestMsg *req,
                                    ProxycCbResponseMsg *rsp)
{
    uint32_t            i, nreqs = req->request_size();
    ProxycCbResponse    *response;

    HAL_TRACE_DEBUG("Rcvd ProxycCb Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::proxyccb_create(spec, response);
    }
    return Status::OK;
}

Status
ProxycCbServiceImpl::ProxycCbUpdate(ServerContext *context,
                                    const ProxycCbRequestMsg *req,
                                    ProxycCbResponseMsg *rsp)
{
    uint32_t            i, nreqs = req->request_size();
    ProxycCbResponse    *response;

    HAL_TRACE_DEBUG("Rcvd ProxycCb Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::proxyccb_update(spec, response);
    }
    return Status::OK;
}


Status
ProxycCbServiceImpl::ProxycCbDelete(ServerContext *context,
                                    const ProxycCbDeleteRequestMsg *req,
                                    ProxycCbDeleteResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();

    HAL_TRACE_DEBUG("Rcvd ProxycCb Delete Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        auto spec = req->request(i);
        hal::proxyccb_delete(spec, rsp);
    }
    return Status::OK;
}

Status
ProxycCbServiceImpl::ProxycCbGet(ServerContext *context,
                                 const ProxycCbGetRequestMsg *req,
                                 ProxycCbGetResponseMsg *rsp)
{
    uint32_t            i, nreqs = req->request_size();
    ProxycCbGetResponse *response;

    HAL_TRACE_DEBUG("Rcvd ProxycCb Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::proxyccb_get(request, response);
    }
    return Status::OK;
}
