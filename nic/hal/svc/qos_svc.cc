//------------------------------------------------------------------------------
// qos service implementation
// Vasanth Kumar (Pensando Systems)
//------------------------------------------------------------------------------
//
#include <base.h>
#include <trace.hpp>
#include <qos_svc.hpp>
#include <qos.hpp>

Status
QOSServiceImpl::BufPoolCreate(ServerContext *context,
                              const BufPoolRequestMsg *req,
                              BufPoolResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    BufPoolResponse      *response;
    hal_ret_t            ret;

    HAL_TRACE_DEBUG("Rcvd BufPool Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        response = rsp->add_response();
        auto spec = req->request(i);
        ret = hal::buf_pool_create(spec, response);
        if (ret == HAL_RET_OK) {
            hal::hal_cfg_db_close(false);
        } else {
            hal::hal_cfg_db_close(true);
        }
    }
    return Status::OK;
}

Status
QOSServiceImpl::BufPoolUpdate(ServerContext *context,
                              const BufPoolRequestMsg *req,
                              BufPoolResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd BufPool Update Request");
    return Status::OK;
}

Status
QOSServiceImpl::BufPoolDelete(ServerContext *context,
                              const BufPoolDeleteRequestMsg *req,
                              BufPoolDeleteResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd BufPool Update Request");
    return Status::OK;
}

Status
QOSServiceImpl::BufPoolGet(ServerContext *context,
                           const BufPoolGetRequestMsg *req,
                           BufPoolGetResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd BufPool Get Request");
    return Status::OK;
}

Status
QOSServiceImpl::QueueCreate(ServerContext *context,
                            const QueueRequestMsg *req,
                            QueueResponseMsg *rsp)
{
    uint32_t           i, nreqs = req->request_size();
    QueueResponse      *response;
    hal_ret_t          ret;

    HAL_TRACE_DEBUG("Rcvd Queue Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        response = rsp->add_response();
        auto spec = req->request(i);
        ret = hal::queue_create(spec, response);
        if (ret == HAL_RET_OK) {
            hal::hal_cfg_db_close(false);
        } else {
            hal::hal_cfg_db_close(true);
        }
    }
    return Status::OK;
}

Status
QOSServiceImpl::QueueUpdate(ServerContext *context,
                              const QueueRequestMsg *req,
                              QueueResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Queue Update Request");
    return Status::OK;
}

Status
QOSServiceImpl::QueueDelete(ServerContext *context,
                              const QueueDeleteRequestMsg *req,
                              QueueDeleteResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Queue Update Request");
    return Status::OK;
}

Status
QOSServiceImpl::QueueGet(ServerContext *context,
                           const QueueGetRequestMsg *req,
                           QueueGetResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Queue Get Request");
    return Status::OK;
}

Status
QOSServiceImpl::PolicerCreate(ServerContext *context,
                              const PolicerRequestMsg *req,
                              PolicerResponseMsg *rsp)
{
    uint32_t           i, nreqs = req->request_size();
    PolicerResponse    *response;
    hal_ret_t          ret;

    HAL_TRACE_DEBUG("Rcvd Policer Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        response = rsp->add_response();
        auto spec = req->request(i);
        ret = hal::policer_create(spec, response);
        if (ret == HAL_RET_OK) {
            hal::hal_cfg_db_close(false);
        } else {
            hal::hal_cfg_db_close(true);
        }
    }
    return Status::OK;
}

Status
QOSServiceImpl::PolicerUpdate(ServerContext *context,
                              const PolicerRequestMsg *req,
                              PolicerResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Policer Update Request");
    return Status::OK;
}

Status
QOSServiceImpl::PolicerDelete(ServerContext *context,
                              const PolicerDeleteRequestMsg *req,
                              PolicerDeleteResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Policer Update Request");
    return Status::OK;
}

Status
QOSServiceImpl::PolicerGet(ServerContext *context,
                           const PolicerGetRequestMsg *req,
                           PolicerGetResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Policer Get Request");
    return Status::OK;
}
