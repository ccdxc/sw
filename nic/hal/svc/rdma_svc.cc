//------------------------------------------------------------------------------
// RDMA service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/rdma_svc.hpp"
#include "nic/hal/src/internal/rdma.hpp"

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
                              const RdmaQpUpdateRequestMsg *req,
                              RdmaQpUpdateResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    RdmaQpUpdateResponse *response;

    HAL_TRACE_DEBUG("Rcvd RdmaQp Update Request");
    if (nreqs == 0) {
        HAL_TRACE_DEBUG("Rcvd RdmaQp Update Request: Zero requests, returning");
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::rdma_qp_update(spec, response);
    }
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
RdmaServiceImpl::RdmaCqCreate(ServerContext *context,
                                const RdmaCqRequestMsg *req,
                                RdmaCqResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    RdmaCqResponse          *response;

    HAL_TRACE_DEBUG("Rcvd RdmaCq Create Request");
    if (nreqs == 0) {
        HAL_TRACE_DEBUG("Rcvd RdmaCq Create Request: Zero requests, returning");
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::rdma_cq_create(spec, response);
    }
    return Status::OK;
}

Status
RdmaServiceImpl::RdmaEqCreate(ServerContext *context,
                                const RdmaEqRequestMsg *req,
                                RdmaEqResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    RdmaEqResponse          *response;

    HAL_TRACE_DEBUG("Rcvd RdmaEq Create Request");
    if (nreqs == 0) {
        HAL_TRACE_DEBUG("Rcvd RdmaEq Create Request: Zero requests, returning");
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::rdma_eq_create(spec, response);
    }
    return Status::OK;
}

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

Status
RdmaServiceImpl::RdmaAhCreate(ServerContext *context,
                              const RdmaAhRequestMsg *req,
                              RdmaAhResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    RdmaAhResponse          *response;

    HAL_TRACE_DEBUG("Rcvd Rdma AH Create Request");
    if (nreqs == 0) {
        HAL_TRACE_DEBUG("Rcvd Rdma AH Create Request: Zero requests, returning");
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::rdma_ah_create(spec, response);
    }
    return Status::OK;
}
