//------------------------------------------------------------------------------
// acl service implementation
// Vasanth Kumar (Pensando Systems)
//------------------------------------------------------------------------------
//
#include <base.h>
#include <trace.hpp>
#include <acl_svc.hpp>
#include <acl.hpp>

Status
AclServiceImpl::AclCreate(ServerContext *context,
                              const AclRequestMsg *req,
                              AclResponseMsg *rsp)
{
    uint32_t       i, nreqs = req->request_size();
    AclResponse    *response;
    hal_ret_t      ret;

    HAL_TRACE_DEBUG("Rcvd Acl Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        response = rsp->add_response();
        auto spec = req->request(i);
        ret = hal::acl_create(spec, response);
        if (ret == HAL_RET_OK) {
            hal::hal_cfg_db_close(false);
        } else {
            hal::hal_cfg_db_close(true);
        }
    }
    return Status::OK;
}

Status
AclServiceImpl::AclUpdate(ServerContext *context,
                              const AclRequestMsg *req,
                              AclResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Acl Update Request");
    return Status::OK;
}

Status
AclServiceImpl::AclDelete(ServerContext *context,
                              const AclDeleteRequestMsg *req_msg,
                              AclDeleteResponseMsg *rsp)
{
    uint32_t            i, nreqs = req_msg->request_size();
    AclDeleteResponse   *response;
    hal_ret_t           ret;

    HAL_TRACE_DEBUG("Rcvd Acl Delete Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        response = rsp->add_response();
        auto req = req_msg->request(i);
        ret = hal::acl_delete(req, response);
        if (ret == HAL_RET_OK) {
            hal::hal_cfg_db_close(false);
        } else {
            hal::hal_cfg_db_close(true);
        }
    }
    return Status::OK;
}

Status
AclServiceImpl::AclGet(ServerContext *context,
                           const AclGetRequestMsg *req,
                           AclGetResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Acl Get Request");
    return Status::OK;
}
