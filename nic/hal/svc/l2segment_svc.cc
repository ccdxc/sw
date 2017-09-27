//------------------------------------------------------------------------------
// L2 segment service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/l2segment_svc.hpp"
#include "nic/hal/src/l2segment.hpp"

Status
L2SegmentServiceImpl::L2SegmentCreate(ServerContext *context,
                                      const L2SegmentRequestMsg *req,
                                      L2SegmentResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    L2SegmentResponse    *response;
    hal_ret_t            ret;

    HAL_TRACE_DEBUG("Rcvd L2Segment Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        response = rsp->add_response();
        auto spec = req->request(i);
        ret = hal::l2segment_create(spec, response);
        if (ret == HAL_RET_OK) {
            hal::hal_cfg_db_close(false);
        } else {
            hal::hal_cfg_db_close(true);
        }
    }
    return Status::OK;
}

Status
L2SegmentServiceImpl::L2SegmentUpdate(ServerContext *context,
                                      const L2SegmentRequestMsg *req,
                                      L2SegmentResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();
    L2SegmentResponse    *response;
    hal_ret_t            ret;

    HAL_TRACE_DEBUG("Rcvd L2Segment Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        response = rsp->add_response();
        auto spec = req->request(i);
        ret = hal::l2segment_update(spec, response);
        if (ret == HAL_RET_OK) {
            hal::hal_cfg_db_close(false);
        } else {
            hal::hal_cfg_db_close(true);
        }
    }
    return Status::OK;
}


Status
L2SegmentServiceImpl::L2SegmentDelete(ServerContext *context,
                                      const L2SegmentDeleteRequestMsg *req,
                                      L2SegmentDeleteResponseMsg *rsp)
{
    uint32_t     i, nreqs = req->request_size();
    hal_ret_t    ret;

    HAL_TRACE_DEBUG("Rcvd L2Segment Delete Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        auto spec = req->request(i);
        ret = hal::l2segment_delete(spec, rsp);
        if (ret == HAL_RET_OK) {
            hal::hal_cfg_db_close(false);
        } else {
            hal::hal_cfg_db_close(true);
        }
    }
    return Status::OK;
}

Status
L2SegmentServiceImpl::L2SegmentGet(ServerContext *context,
                                   const L2SegmentGetRequestMsg *req,
                                   L2SegmentGetResponseMsg *rsp)
{
    uint32_t                i, nreqs = req->request_size();
    L2SegmentGetResponse    *response;

    HAL_TRACE_DEBUG("Rcvd L2Segment Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto request = req->request(i);
        hal::l2segment_get(request, response);
    }
    hal::hal_cfg_db_close(true);
    return Status::OK;
}
