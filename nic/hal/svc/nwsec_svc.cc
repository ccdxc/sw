//------------------------------------------------------------------------------
// network security service implementation
//------------------------------------------------------------------------------

#include <base.h>
#include <trace.hpp>
#include <nwsec_svc.hpp>
#include <nwsec.hpp>

Status
NwSecurityServiceImpl::SecurityProfileCreate(ServerContext *context,
                                             const SecurityProfileRequestMsg *req,
                                             SecurityProfileResponseMsg *rsp)
{
    uint32_t                   i, nreqs = req->request_size();
    SecurityProfileResponse    *response;

    HAL_TRACE_DEBUG("Rcvd SecurityProfile Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::security_profile_create(spec, response);
    }
    return Status::OK;
}

Status
NwSecurityServiceImpl::SecurityProfileUpdate(ServerContext *context,
                                             const SecurityProfileRequestMsg *req,
                                             SecurityProfileResponseMsg *rsp)
{
    uint32_t                   i, nreqs = req->request_size();
    SecurityProfileResponse    *response;

    HAL_TRACE_DEBUG("Rcvd SecurityProfile Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::security_profile_update(spec, response);
    }
    return Status::OK;
}


Status
NwSecurityServiceImpl::SecurityProfileDelete(ServerContext *context,
                                             const SecurityProfileDeleteRequestMsg *req,
                                             SecurityProfileDeleteResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd SecurityProfile Delete Request");
    return Status::OK;
}

Status
NwSecurityServiceImpl::SecurityProfileGet(ServerContext *context,
                                          const SecurityProfileGetRequestMsg *req,
                                          SecurityProfileGetResponseMsg *rsp)
{
    uint32_t                      i, nreqs = req->request_size();
    SecurityProfileGetResponse    *response;

    HAL_TRACE_DEBUG("Rcvd SecurityProfile Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::security_profile_get(request, response);
    }
    return Status::OK;
}
