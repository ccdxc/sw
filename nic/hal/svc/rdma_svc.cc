//------------------------------------------------------------------------------
// RDMA service implementation
//------------------------------------------------------------------------------

#include <base.h>
#include <trace.hpp>
#include <rdma_svc.hpp>
#include <rdma.hpp>

Status
RdmaServiceImpl::RdmaQpCreate(ServerContext *context,
                                const RdmaQpRequestMsg *req,
                                RdmaQpResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    RdmaQpResponse          *response;

    HAL_TRACE_DEBUG("Rcvd RdmaQp Create Request");
    if (nreqs == 0) {
        HAL_TRACE_DEBUG("Rcvd RdmaQp Create Request: Zero requests, returning");
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::rdma_qp_create(spec, response);
    }
    return Status::OK;
}

Status
RdmaServiceImpl::RdmaQpUpdate(ServerContext *context,
                                const RdmaQpRequestMsg *req,
                                RdmaQpResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd RdmaQp Update Request");
    return Status::OK;
}

#if 0
Status
RdmaServiceImpl::RdmaQpDelete(ServerContext *context,
                                const RdmaQpDeleteRequestMsg *req,
                                RdmaQpDeleteResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd RdmaQp Update Request");
    return Status::OK;
}

Status
RdmaServiceImpl::RdmaQpGet(ServerContext *context,
                             const RdmaQpGetRequestMsg *req,
                             RdmaQpGetResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd RdmaQp Get Request");
    return Status::OK;
}
#endif

Status
RdmaServiceImpl::RdmaMemReg(ServerContext *context,
                            const RdmaMemRegRequestMsg *req,
                            RdmaMemRegResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    RdmaMemRegResponse   *response;

    HAL_TRACE_DEBUG("Rcvd RDMA Memory Registration Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::rdma_memory_register(spec, response);
    }
    return Status::OK;
}
