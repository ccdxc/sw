//------------------------------------------------------------------------------
// Multicast service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/multicast_svc.hpp"
#include "nic/hal/src/multicast.hpp"

Status
MulticastServiceImpl::MulticastEntryCreate(ServerContext *context,
                                           const MulticastEntryRequestMsg *req,
                                           MulticastEntryResponseMsg *rsp)
{
    uint32_t             i, nreqs = (uint32_t)req->request_size();
    MulticastEntryResponse *response;

    HAL_TRACE_DEBUG("Rcvd MulticastEntry Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::multicast_entry_create(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
MulticastServiceImpl::MulticastEntryUpdate(ServerContext *context,
                                           const MulticastEntryRequestMsg *req,
                                           MulticastEntryResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    MulticastEntryResponse    *response;

    HAL_TRACE_DEBUG("Rcvd MulticastEntry Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::multicast_entry_update(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
MulticastServiceImpl::MulticastEntryDelete(ServerContext *context,
                                           const MulticastEntryDeleteRequestMsg *req,
                                           MulticastEntryDeleteResponseMsg *rsp)
{
    uint32_t     i, nreqs = req->request_size();
    MulticastEntryDeleteResponse    *response;

    HAL_TRACE_DEBUG("Rcvd MulticastEntry Delete Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::multicast_entry_delete(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
MulticastServiceImpl::MulticastEntryGet(ServerContext *context,
                                        const MulticastEntryGetRequestMsg *req,
                                        MulticastEntryGetResponseMsg *rsp)
{
    uint32_t                i, nreqs = req->request_size();

    HAL_TRACE_DEBUG("Rcvd MulticastEntry Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        hal::multicast_entry_get(request, rsp);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
