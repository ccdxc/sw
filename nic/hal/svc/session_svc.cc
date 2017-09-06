//------------------------------------------------------------------------------
// session service implementation
//------------------------------------------------------------------------------

#include <base.h>
#include <trace.hpp>
#include <session_svc.hpp>
#include <session.hpp>
#include <fte.hpp>

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

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::session_create(spec, response);
    }
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

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::session_get(spec, response);
    }
    return Status::OK;
}
