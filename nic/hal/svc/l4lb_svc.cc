//------------------------------------------------------------------------------
// L4 LB service implementation
//------------------------------------------------------------------------------

#include <base.h>
#include <trace.hpp>
#include <l4lb_svc.hpp>
#include <l4lb.pb.h>
#include <l4lb.hpp>

Status
L4LbServiceImpl::L4LbServiceCreate(ServerContext *context,
                                   const L4LbServiceRequestMsg *req,
                                   L4LbServiceResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    l4lb::L4LbServiceResponse  *response;

    HAL_TRACE_DEBUG("Rcvd L2Segment Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::l4lbservice_create(spec, response);
    }
    return Status::OK;
}

Status
L4LbServiceImpl::L4LbServiceUpdate(ServerContext *context,
                                   const L4LbServiceRequestMsg *req,
                                   L4LbServiceResponseMsg *rsp)
{
    return Status::OK;
}

Status
L4LbServiceImpl::L4LbServiceDelete(ServerContext *context,
                                   const L4LbServiceDeleteRequestMsg *req,
                                   L4LbServiceDeleteResponseMsg *rsp)
{
    return Status::OK;
}

Status
L4LbServiceImpl::L4LbServiceGet(ServerContext *context,
                                const L4LbServiceGetRequestMsg *req,
                                L4LbServiceGetResponseMsg *rsp)
{
    return Status::OK;
}

Status
L4LbServiceImpl::L4LbBackendCreate(ServerContext *context,
                                   const L4LbBackendRequestMsg *req,
                                   L4LbBackendResponseMsg *rsp)
{
    return Status::OK;
}

Status
L4LbServiceImpl::L4LbBackendUpdate(ServerContext *context,
                                   const L4LbBackendRequestMsg *req,
                                   L4LbBackendResponseMsg *rsp)
{
    return Status::OK;
}

Status
L4LbServiceImpl::L4LbBackendDelete(ServerContext *context,
                                   const L4LbBackendDeleteRequestMsg *req,
                                   L4LbBackendDeleteResponseMsg *rsp)
{
    return Status::OK;
}


Status
L4LbServiceImpl::L4LbBackendGet(ServerContext *context,
                                const L4LbBackendGetRequestMsg *req,
                                L4LbBackendGetResponseMsg *rsp)
{
    return Status::OK;
}

