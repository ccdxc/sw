//------------------------------------------------------------------------------
// interface service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/interface_svc.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/lif.hpp"

Status
InterfaceServiceImpl::LifCreate(ServerContext *context,
                                const LifRequestMsg *req,
                                LifResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    LifResponse          *response;

    HAL_TRACE_DEBUG("Received lif Create");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::lif_create(spec, response, NULL);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
InterfaceServiceImpl::LifUpdate(ServerContext *context,
                                const LifRequestMsg *req,
                                LifResponseMsg *rsp)
{
    uint32_t          i, nreqs = req->request_size();
    LifResponse       *response;

    HAL_TRACE_DEBUG("Received lif update");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::lif_update(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
InterfaceServiceImpl::LifDelete(ServerContext *context,
                                const LifDeleteRequestMsg *req,
                                LifDeleteResponseMsg *rsp)
{
    uint32_t     i, nreqs = req->request_size();
    LifDeleteResponse     *response;

    HAL_TRACE_DEBUG("Received lif delete");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::lif_delete(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
InterfaceServiceImpl::LifGet(ServerContext *context,
                             const LifGetRequestMsg *req,
                             LifGetResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();

    HAL_TRACE_DEBUG("Received lif get");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        hal::lif_get(request, rsp);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
InterfaceServiceImpl::LifGetQState(ServerContext *context,
                                   const GetQStateRequestMsg *reqs,
                                   GetQStateResponseMsg *resps)
{
    HAL_TRACE_DEBUG("Rcvd Get QState Request");
    for (int i = 0; i < reqs->reqs_size(); i++) {
        hal::LifGetQState(reqs->reqs(i), resps->add_resps());
    }
    return Status::OK;
}

Status
InterfaceServiceImpl::LifSetQState(ServerContext *context,
                                   const SetQStateRequestMsg *reqs,
                                   SetQStateResponseMsg *resps)
{
    HAL_TRACE_DEBUG("Rcvd Get QState Request");
    for (int i = 0; i < reqs->reqs_size(); i++) {
        hal::LifSetQState(reqs->reqs(i), resps->add_resps());
    }
    return Status::OK;
}

Status
InterfaceServiceImpl::InterfaceCreate(ServerContext* context,
                                      const InterfaceRequestMsg *req,
                                      InterfaceResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    InterfaceResponse    *response;

    HAL_TRACE_DEBUG("Received if create");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::interface_create(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
InterfaceServiceImpl::InterfaceUpdate(ServerContext* context,
                                      const InterfaceRequestMsg *req,
                                      InterfaceResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    InterfaceResponse    *response;

    HAL_TRACE_DEBUG("Received if update");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::interface_update(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
InterfaceServiceImpl::InterfaceDelete(ServerContext *context,
                                      const InterfaceDeleteRequestMsg *req,
                                      InterfaceDeleteResponseMsg *rsp)
{
    uint32_t     i, nreqs = req->request_size();
    InterfaceDeleteResponse     *response;

    HAL_TRACE_DEBUG("Received if delete");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::interface_delete(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
InterfaceServiceImpl::InterfaceGet(ServerContext *context,
                                   const InterfaceGetRequestMsg *req,
                                   InterfaceGetResponseMsg *rsp)
{
    uint32_t                i, nreqs = req->request_size();

    HAL_TRACE_DEBUG("Received if get ");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    for (i = 0; i < nreqs; i++) {
        auto spec = req->request(i);
        hal::interface_get(spec, rsp);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
InterfaceServiceImpl::AddL2SegmentOnUplink(ServerContext *context,
                                           const InterfaceL2SegmentRequestMsg *req,
                                           InterfaceL2SegmentResponseMsg *rsp)
{
    uint32_t                        i, nreqs = req->request_size();
    InterfaceL2SegmentResponse      *response;

    HAL_TRACE_DEBUG("Received add l2seg on uplink");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::add_l2seg_on_uplink(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
InterfaceServiceImpl::DelL2SegmentOnUplink(ServerContext *context,
                                           const InterfaceL2SegmentRequestMsg *req,
                                           InterfaceL2SegmentResponseMsg *rsp)
{
    uint32_t                        i, nreqs = req->request_size();
    InterfaceL2SegmentResponse      *response;

    HAL_TRACE_DEBUG("Received delete l2seg on uplink");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::del_l2seg_on_uplink(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
