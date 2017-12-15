//------------------------------------------------------------------------------
// qos service implementation
// Vasanth Kumar (Pensando Systems)
//------------------------------------------------------------------------------
//
#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/qos_svc.hpp"
#include "nic/hal/src/qos.hpp"

Status
QOSServiceImpl::QosClassCreate(ServerContext *context,
                            const QosClassRequestMsg *req,
                            QosClassResponseMsg *rsp)
{
    uint32_t           i, nreqs = req->request_size();
    QosClassResponse   *response;

    HAL_TRACE_DEBUG("Rcvd QosClass Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::qos_class_create(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
QOSServiceImpl::QosClassUpdate(ServerContext *context,
                              const QosClassRequestMsg *req,
                              QosClassResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd QosClass Update Request");
    return Status::OK;
}

Status
QOSServiceImpl::QosClassDelete(ServerContext *context,
                              const QosClassDeleteRequestMsg *req,
                              QosClassDeleteResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd QosClass Update Request");
    return Status::OK;
}

Status
QOSServiceImpl::QosClassGet(ServerContext *context,
                           const QosClassGetRequestMsg *req,
                           QosClassGetResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd QosClass Get Request");
    return Status::OK;
}
