//------------------------------------------------------------------------------
// interface service implementation
//------------------------------------------------------------------------------

#include <base.h>
#include <trace.hpp>
#include <interface_svc.hpp>
#include <interface.hpp>

Status
InterfaceServiceImpl::LifCreate(ServerContext *context,
                                const LifRequestMsg *req,
                                LifResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    LifResponse          *response;

    HAL_TRACE_DEBUG("Rcvd Lif Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::lif_create(spec, response, NULL);
    }
    return Status::OK;
}

Status
InterfaceServiceImpl::LifUpdate(ServerContext *context,
                                const LifRequestMsg *req,
                                LifResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Lif Update Request");
    return Status::OK;
}

Status
InterfaceServiceImpl::LifDelete(ServerContext *context,
                                const LifDeleteRequestMsg *req,
                                LifDeleteResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Lif Update Request");
    return Status::OK;
}

Status
InterfaceServiceImpl::LifGet(ServerContext *context,
                             const LifGetRequestMsg *req,
                             LifGetResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Lif Get Request");
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

    HAL_TRACE_DEBUG("Rcvd Interface Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::interface_create(spec, response);
    }
    return Status::OK;
}

Status
InterfaceServiceImpl::InterfaceUpdate(ServerContext* context,
                                      const InterfaceRequestMsg *req,
                                      InterfaceResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    InterfaceResponse    *response;

    HAL_TRACE_DEBUG("Rcvd Interface Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::interface_update(spec, response);
    }
    return Status::OK;
}

Status
InterfaceServiceImpl::InterfaceDelete(ServerContext *context,
                                      const InterfaceDeleteRequestMsg *req,
                                      InterfaceDeleteResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Interface Delete Request");
    return Status::OK;
}

Status
InterfaceServiceImpl::InterfaceGet(ServerContext *context,
                                   const InterfaceGetRequestMsg *req,
                                   InterfaceGetResponseMsg *rsp)
{
    uint32_t                i, nreqs = req->request_size();
    InterfaceGetResponse    *response;

    HAL_TRACE_DEBUG("Rcvd Interface Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::interface_get(spec, response);
    }
    return Status::OK;
}

Status
InterfaceServiceImpl::AddL2SegmentOnUplink(ServerContext *context,
                                           const InterfaceL2SegmentRequestMsg *req,
                                           InterfaceL2SegmentResponseMsg *rsp)
{
    uint32_t                        i, nreqs = req->request_size();
    InterfaceL2SegmentResponse      *response;

    HAL_TRACE_DEBUG("Rcvd Add L2Segment to Uplink Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::add_l2seg_on_uplink(spec, response);
    }
    return Status::OK;
}

Status
InterfaceServiceImpl::DelL2SegmentOnUplink(ServerContext *context,
                                           const InterfaceL2SegmentRequestMsg *req,
                                           InterfaceL2SegmentResponseMsg *rsp)
{
    uint32_t                        i, nreqs = req->request_size();
    InterfaceL2SegmentResponse      *response;

    HAL_TRACE_DEBUG("Rcvd Del L2Segment to Uplink Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::del_l2seg_on_uplink(spec, response);
    }
    return Status::OK;
}
