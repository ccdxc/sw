//------------------------------------------------------------------------------
// network security service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/nwsec_svc.hpp"
#include "nic/hal/src/nwsec.hpp"

Status
NwSecurityServiceImpl::SecurityProfileCreate(ServerContext *context,
                                             const SecurityProfileRequestMsg *req,
                                             SecurityProfileResponseMsg *rsp)
{
    uint32_t                   i, nreqs = req->request_size();
    SecurityProfileResponse    *response;
    hal_ret_t                   ret;

    HAL_TRACE_DEBUG("Rcvd SecurityProfile Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        response = rsp->add_response();
        auto spec = req->request(i);
        ret = hal::security_profile_create(spec, response);
        if (ret == HAL_RET_OK) {
            hal::hal_cfg_db_close(false);
        } else {
            hal::hal_cfg_db_close(true);
        }
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
    hal_ret_t                  ret;

    HAL_TRACE_DEBUG("Rcvd SecurityProfile Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        response = rsp->add_response();
        auto spec = req->request(i);
        ret = hal::security_profile_update(spec, response);
        if (ret == HAL_RET_OK) {
            hal::hal_cfg_db_close(false);
        } else {
            hal::hal_cfg_db_close(true);
        }
    }
    return Status::OK;
}


Status
NwSecurityServiceImpl::SecurityProfileDelete(ServerContext *context,
                                             const SecurityProfileDeleteRequestMsg *req,
                                             SecurityProfileDeleteResponseMsg *rsp)
{
    uint32_t     i, nreqs = req->request_size();
    hal_ret_t    ret;

    HAL_TRACE_DEBUG("Rcvd SecurityProfile Delete Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        auto spec = req->request(i);
        ret = hal::security_profile_delete(spec, rsp);
        if (ret == HAL_RET_OK) {
            hal::hal_cfg_db_close(false);
        } else {
            hal::hal_cfg_db_close(true);
        }
    }
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
    hal::hal_cfg_db_close(true);
    return Status::OK;
}


Status
NwSecurityServiceImpl::SecurityGroupCreate(ServerContext *context,
                                             const SecurityGroupRequestMsg *req,
                                             SecurityGroupResponseMsg *rsp)
{
    uint32_t                    i, nreqs = req->request_size();
    SecurityGroupResponse       *response;
    hal_ret_t                   ret;

    HAL_TRACE_DEBUG("Rcvd SecurityProfile Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        response = rsp->add_response();
        auto spec = req->request(i);
        ret = hal::security_group_create(spec, response);
        if (ret == HAL_RET_OK) {
            hal::hal_cfg_db_close(false);
        } else {
            hal::hal_cfg_db_close(true);
        }
    }
    return Status::OK;
}

Status
NwSecurityServiceImpl::SecurityGroupUpdate(ServerContext *context,
                                             const SecurityGroupRequestMsg *req,
                                             SecurityGroupResponseMsg *rsp)
{
    uint32_t                    i, nreqs = req->request_size();
    SecurityGroupResponse       *response;
    hal_ret_t                   ret;

    HAL_TRACE_DEBUG("Rcvd SecurityGroup Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        response = rsp->add_response();
        auto spec = req->request(i);
        ret = hal::security_group_update(spec, response);
        if (ret == HAL_RET_OK) {
            hal::hal_cfg_db_close(false);
        } else {
            hal::hal_cfg_db_close(true);
        }
    }
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
    hal::hal_cfg_db_close(true);
    return Status::OK;
}
