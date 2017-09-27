//------------------------------------------------------------------------------
// IPSEC Cb service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/ipseccb_svc.hpp"
#include "nic/hal/src/ipseccb.hpp"

Status
IpsecCbServiceImpl::IpsecCbCreate(ServerContext *context,
                              const IpsecCbRequestMsg *req,
                              IpsecCbResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();
    IpsecCbResponse    *response;

    HAL_TRACE_DEBUG("Rcvd IpsecCb Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::ipseccb_create(spec, response);
    }
    return Status::OK;
}

Status
IpsecCbServiceImpl::IpsecCbUpdate(ServerContext *context,
                              const IpsecCbRequestMsg *req,
                              IpsecCbResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();
    IpsecCbResponse    *response;

    HAL_TRACE_DEBUG("Rcvd IpsecCb Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::ipseccb_update(spec, response);
    }
    return Status::OK;
}


Status
IpsecCbServiceImpl::IpsecCbDelete(ServerContext *context,
                              const IpsecCbDeleteRequestMsg *req,
                              IpsecCbDeleteResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();

    HAL_TRACE_DEBUG("Rcvd IpsecCb Delete Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        auto spec = req->request(i);
        hal::ipseccb_delete(spec, rsp);
    }
    return Status::OK;
}

Status
IpsecCbServiceImpl::IpsecCbGet(ServerContext *context,
                           const IpsecCbGetRequestMsg *req,
                           IpsecCbGetResponseMsg *rsp)
{
    uint32_t            i, nreqs = req->request_size();
    IpsecCbGetResponse    *response;

    HAL_TRACE_DEBUG("Rcvd IpsecCb Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::ipseccb_get(request, response);
    }
    return Status::OK;
}
