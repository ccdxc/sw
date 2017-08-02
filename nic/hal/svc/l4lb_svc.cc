//------------------------------------------------------------------------------
// L4 LB service implementation
//------------------------------------------------------------------------------

#include <base.h>
#include <trace.hpp>
#include <l4lb_svc.hpp>

Status
L4LbServiceImpl::L4LbServiceCreate(ServerContext *context,
                                   const L4LbServiceRequestMsg *req,
                                   L4LbServiceResponseMsg *rsp)
{
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

