// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

//------------------------------------------------------------------------------
// GFT service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/gft_svc.hpp"
#include "nic/hal/src/gft.hpp"

Status
GftServiceImpl::GftExactMatchProfileCreate(ServerContext *context,
                                      const GftExactMatchProfileRequestMsg *req,
                                      GftExactMatchProfileResponseMsg *rsp)
{
    uint32_t                        i, nreqs = req->request_size();
    GftExactMatchProfileResponse    *response;

    HAL_TRACE_DEBUG("Rcvd GFT exact match profile create request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::gft_exact_match_profile_create(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
GftServiceImpl::GftHeaderTranspositionProfileCreate(ServerContext *context,
                                      const GftHeaderTranspositionProfileRequestMsg *req,
                                      GftHeaderTranspositionProfileResponseMsg *rsp)
{
    uint32_t                                 i, nreqs = req->request_size();
    GftHeaderTranspositionProfileResponse    *response;

    HAL_TRACE_DEBUG("Rcvd GFT header transposition profile create request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::gft_header_transposition_profile_create(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
GftServiceImpl::GftExactMatchFlowEntryCreate(ServerContext *context,
                                      const GftExactMatchFlowEntryRequestMsg *req,
                                      GftExactMatchFlowEntryResponseMsg *rsp)
{
    uint32_t                          i, nreqs = req->request_size();
    GftExactMatchFlowEntryResponse    *response;

    HAL_TRACE_DEBUG("Rcvd GFT exact match flow entry create request");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::gft_exact_match_flow_entry_create(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
