//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// vrf service implementation
//------------------------------------------------------------------------------

#include "nic/include/hal_cfg.hpp"
#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/vrf_svc.hpp"
#include "nic/hal/src/vrf.hpp"

Status
VrfServiceImpl::VrfCreate(ServerContext *context,
                                const VrfRequestMsg *req,
                                VrfResponseMsg *rsp)
{
    uint32_t          i, nreqs = req->request_size();
    VrfResponse    *response;

    HAL_TRACE_DEBUG("Rcvd Vrf Create Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::vrf_create(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
VrfServiceImpl::VrfUpdate(ServerContext *context,
                                const VrfRequestMsg *req,
                                VrfResponseMsg *rsp)
{
    uint32_t          i, nreqs = req->request_size();
    VrfResponse    *response;

    HAL_TRACE_DEBUG("Rcvd Vrf Update Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::vrf_update(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}


Status
VrfServiceImpl::VrfDelete(ServerContext *context,
                                const VrfDeleteRequestMsg *req,
                                VrfDeleteResponseMsg *rsp)
{
    uint32_t     i, nreqs = req->request_size();
    VrfDeleteResponse    *response;

    HAL_TRACE_DEBUG("Rcvd Vrf Delete Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::vrf_delete(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
VrfServiceImpl::VrfGet(ServerContext *context,
                             const VrfGetRequestMsg *req,
                             VrfGetResponseMsg *rsp)
{
    uint32_t             i, nreqs = req->request_size();

    HAL_TRACE_DEBUG("Rcvd Vrf Get Request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        hal::vrf_get(request, rsp);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
