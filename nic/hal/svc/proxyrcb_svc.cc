//------------------------------------------------------------------------------
// PROXYRCB (L7 Proxy Redirect CB) service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/proxyrcb_svc.hpp"
#include "nic/hal/src/proxyrcb.hpp"

Status
ProxyrCbServiceImpl::ProxyrCbCreate(ServerContext *context,
                                    const ProxyrCbRequestMsg *req,
                                    ProxyrCbResponseMsg *rsp)
{
    uint32_t            i, nreqs = req->request_size();
    ProxyrCbResponse    *response;

    HAL_TRACE_DEBUG("Rcvd ProxyrCb Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::proxyrcb_create(spec, response);
    }
    return Status::OK;
}

Status
ProxyrCbServiceImpl::ProxyrCbUpdate(ServerContext *context,
                                    const ProxyrCbRequestMsg *req,
                                    ProxyrCbResponseMsg *rsp)
{
    uint32_t            i, nreqs = req->request_size();
    ProxyrCbResponse    *response;

    HAL_TRACE_DEBUG("Rcvd ProxyrCb Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::proxyrcb_update(spec, response);
    }
    return Status::OK;
}


Status
ProxyrCbServiceImpl::ProxyrCbDelete(ServerContext *context,
                                    const ProxyrCbDeleteRequestMsg *req,
                                    ProxyrCbDeleteResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();

    HAL_TRACE_DEBUG("Rcvd ProxyrCb Delete Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        auto spec = req->request(i);
        hal::proxyrcb_delete(spec, rsp);
    }
    return Status::OK;
}

Status
ProxyrCbServiceImpl::ProxyrCbGet(ServerContext *context,
                                 const ProxyrCbGetRequestMsg *req,
                                 ProxyrCbGetResponseMsg *rsp)
{
    uint32_t            i, nreqs = req->request_size();
    ProxyrCbGetResponse *response;

    HAL_TRACE_DEBUG("Rcvd ProxyrCb Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::proxyrcb_get(request, response);
    }
    return Status::OK;
}
