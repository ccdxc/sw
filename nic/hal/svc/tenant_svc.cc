//------------------------------------------------------------------------------
// tenant service implementation
//------------------------------------------------------------------------------

#include <base.h>
#include <trace.hpp>
#include <hal_cfg.hpp>
#include <tenant_svc.hpp>
#include <tenant.hpp>

Status
TenantServiceImpl::TenantCreate(ServerContext *context,
                                const TenantRequestMsg *req,
                                TenantResponseMsg *rsp)
{
    uint32_t          i, nreqs = req->request_size();
    TenantResponse    *response;

    HAL_TRACE_DEBUG("Rcvd Tenant Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::tenant_create(spec, response);
    }
    hal::hal_cfg_db_close(false);

    return Status::OK;
}

Status
TenantServiceImpl::TenantUpdate(ServerContext *context,
                                const TenantRequestMsg *req,
                                TenantResponseMsg *rsp)
{
    uint32_t          i, nreqs = req->request_size();
    TenantResponse    *response;

    HAL_TRACE_DEBUG("Rcvd Tenant Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::tenant_update(spec, response);
    }
    hal::hal_cfg_db_close(false);

    return Status::OK;
}


Status
TenantServiceImpl::TenantDelete(ServerContext *context,
                                const TenantDeleteRequestMsg *req,
                                TenantDeleteResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Tenant Delete Request");
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    hal::hal_cfg_db_close(false);
    return Status::OK;
}

Status
TenantServiceImpl::TenantGet(ServerContext *context,
                             const TenantGetRequestMsg *req,
                             TenantGetResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    TenantGetResponse    *response;

    HAL_TRACE_DEBUG("Rcvd Tenant Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::tenant_get(request, response);
    }
    hal::hal_cfg_db_close(false);
    return Status::OK;
}
