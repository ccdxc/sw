//------------------------------------------------------------------------------
// TCP Cb service implementation
//------------------------------------------------------------------------------

#include <base.h>
#include <trace.hpp>
#include <tcpcb_svc.hpp>
#include <tcpcb.hpp>

Status
TcpCbServiceImpl::TcpCbCreate(ServerContext *context,
                              const TcpCbRequestMsg *req,
                              TcpCbResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();
    TcpCbResponse    *response;

    HAL_TRACE_DEBUG("Rcvd TcpCb Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::tcpcb_create(spec, response);
    }
    return Status::OK;
}

Status
TcpCbServiceImpl::TcpCbUpdate(ServerContext *context,
                              const TcpCbRequestMsg *req,
                              TcpCbResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();
    TcpCbResponse    *response;

    HAL_TRACE_DEBUG("Rcvd TcpCb Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::tcpcb_update(spec, response);
    }
    return Status::OK;
}


Status
TcpCbServiceImpl::TcpCbDelete(ServerContext *context,
                              const TcpCbDeleteRequestMsg *req,
                              TcpCbDeleteResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd TcpCb Delete Request");
    return Status::OK;
}

Status
TcpCbServiceImpl::TcpCbGet(ServerContext *context,
                           const TcpCbGetRequestMsg *req,
                           TcpCbGetResponseMsg *rsp)
{
    uint32_t            i, nreqs = req->request_size();
    TcpCbGetResponse    *response;

    HAL_TRACE_DEBUG("Rcvd TcpCb Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::tcpcb_get(request, response);
    }
    return Status::OK;
}
