//------------------------------------------------------------------------------
// network security service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/nwsec_svc.hpp"
#include "nic/hal/src/nwsec.hpp"
#include "nic/hal/src/dos.hpp"

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

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::security_profile_create(spec, response);
    }
    hal::hal_cfg_db_close();
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

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::security_profile_update(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}


Status
NwSecurityServiceImpl::SecurityProfileDelete(ServerContext *context,
                                             const SecurityProfileDeleteRequestMsg *req,
                                             SecurityProfileDeleteResponseMsg *rsp)
{
    uint32_t     i, nreqs = req->request_size();
    SecurityProfileDeleteResponse    *response;

    HAL_TRACE_DEBUG("Rcvd SecurityProfile Delete Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::security_profile_delete(spec, response);
    }
    hal::hal_cfg_db_close();
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

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::security_profile_get(request, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

/*  Security Group Policy Implementation
 */
Status
NwSecurityServiceImpl::SecurityGroupPolicyCreate(ServerContext                       *context,
                                                 const SecurityGroupPolicyRequestMsg *req,
                                                 SecurityGroupPolicyResponseMsg      *rsp)
{
    uint32_t                           i, nreqs = req->request_size();
    SecurityGroupPolicyResponse       *response;

    HAL_TRACE_DEBUG("Rcvd SecurityGroupPolicy Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::security_group_policy_create(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
NwSecurityServiceImpl::SecurityGroupPolicyUpdate(ServerContext                       *context,
                                                 const SecurityGroupPolicyRequestMsg *req,
                                                 SecurityGroupPolicyResponseMsg      *rsp)
{
    uint32_t                          i, nreqs = req->request_size();
    SecurityGroupPolicyResponse       *response;

    HAL_TRACE_DEBUG("Rcvd SecurityGroupPolicy Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::security_group_policy_update(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}


Status
NwSecurityServiceImpl::SecurityGroupPolicyDelete(ServerContext                             *context,
                                                 const SecurityGroupPolicyDeleteRequestMsg *req,
                                                 SecurityGroupPolicyDeleteResponseMsg      *rsp)
{
    HAL_TRACE_DEBUG("Rcvd SecuritygroupPolicy Delete Request");
    return Status::OK;
}

Status
NwSecurityServiceImpl::SecurityGroupPolicyGet(ServerContext                          *context,
                                              const SecurityGroupPolicyGetRequestMsg *req,
                                              SecurityGroupPolicyGetResponseMsg      *rsp)
{
    uint32_t                          i, nreqs = req->request_size();
    SecurityGroupPolicyGetResponse    *response;

    HAL_TRACE_DEBUG("Rcvd SecurityGroupPolicy Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::security_group_policy_get(request, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

/* Security Group create
*/
Status
NwSecurityServiceImpl::SecurityGroupCreate(ServerContext *context,
                                           const SecurityGroupRequestMsg *req,
                                           SecurityGroupResponseMsg *rsp)
{
    uint32_t                    i, nreqs = req->request_size();
    SecurityGroupResponse       *response;

    HAL_TRACE_DEBUG("Rcvd SecurityGroup Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::security_group_create(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
NwSecurityServiceImpl::SecurityGroupUpdate(ServerContext *context,
                                           const SecurityGroupRequestMsg *req,
                                           SecurityGroupResponseMsg *rsp)
{
    uint32_t                    i, nreqs = req->request_size();
    SecurityGroupResponse       *response;

    HAL_TRACE_DEBUG("Rcvd SecurityGroup Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::security_group_update(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}


Status
NwSecurityServiceImpl::SecurityGroupDelete(ServerContext *context,
                                           const SecurityGroupDeleteRequestMsg *req,
                                           SecurityGroupDeleteResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Securitygroup Delete Request");
    return Status::OK;
}

Status
NwSecurityServiceImpl::SecurityGroupGet(ServerContext *context,
                                        const SecurityGroupGetRequestMsg *req,
                                        SecurityGroupGetResponseMsg *rsp)
{
    uint32_t                      i, nreqs = req->request_size();
    SecurityGroupGetResponse    *response;

    HAL_TRACE_DEBUG("Rcvd SecurityGroup Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::security_group_get(request, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

/*
 * Denial-of-Service policy implementation
 */
Status
NwSecurityServiceImpl::DoSPolicyCreate(ServerContext *context,
                                       const DoSPolicyRequestMsg *req,
                                       DoSPolicyResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    DoSPolicyResponse    *response;

    HAL_TRACE_DEBUG("Rcvd DoSPolicy Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::dos_policy_create(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
NwSecurityServiceImpl::DoSPolicyUpdate(ServerContext *context,
                                       const DoSPolicyRequestMsg *req,
                                       DoSPolicyResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    DoSPolicyResponse    *response;

    HAL_TRACE_DEBUG("Rcvd DoSPolicy Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::dos_policy_update(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}


Status
NwSecurityServiceImpl::DoSPolicyDelete(ServerContext *context,
                                       const DoSPolicyDeleteRequestMsg *req,
                                       DoSPolicyDeleteResponseMsg *rsp)
{
    uint32_t     i, nreqs = req->request_size();

    HAL_TRACE_DEBUG("Rcvd DoSPolicy Delete Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        auto spec = req->request(i);
        hal::dos_policy_delete(spec, rsp);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
NwSecurityServiceImpl::DoSPolicyGet(ServerContext *context,
                                    const DoSPolicyGetRequestMsg *req,
                                    DoSPolicyGetResponseMsg *rsp)
{
    uint32_t                i, nreqs = req->request_size();
    DoSPolicyGetResponse    *response;

    HAL_TRACE_DEBUG("Rcvd DoSPolicy Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::dos_policy_get(request, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
