//------------------------------------------------------------------------------
// session service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/session_svc.hpp"
#include "nic/hal/src/nw/session.hpp"
#include "nic/include/fte.hpp"

Status
SessionServiceImpl::SessionCreate(ServerContext *context,
                                  const SessionRequestMsg *req,
                                  SessionResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    SessionResponse      *response;

    HAL_TRACE_DEBUG("Rcvd Session Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        fte::session_create(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
SessionServiceImpl::SessionDelete(ServerContext *context,
                                  const SessionDeleteRequestMsg *req,
                                  SessionDeleteResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Session Delete Request");
    return Status::OK;
}

Status
SessionServiceImpl::SessionGet(ServerContext *context,
                               const SessionGetRequestMsg *req,
                               SessionGetResponseMsg *rsp)
{
    uint32_t                i, nreqs = req->request_size();
    SessionGetResponse      *response;

    HAL_TRACE_DEBUG("Rcvd Session Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::session_get(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
