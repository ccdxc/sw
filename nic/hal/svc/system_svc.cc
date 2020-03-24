//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// sys service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal_trace.hpp"
#include "nic/include/hal.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/include/hal_cfg_db.hpp"
#include "nic/hal/svc/system_svc.hpp"
#include "gen/proto/system.pb.h"
#include "gen/proto/types.pb.h"
#include "nic/hal/src/internal/system.hpp"

Status
SystemServiceImpl::APIStatsGet(ServerContext *context,
                               const Empty *request,
                               ApiStatsResponse *rsp)
{
    HAL_TRACE_DEBUG("Rcvd API Stats Get Request");
    hal::api_stats_get(rsp);
    return Status::OK;
}

Status
SystemServiceImpl::SystemGet(ServerContext *context,
                             const SystemGetRequest *request,
                             SystemResponse *rsp)
{
    HAL_TRACE_DEBUG("Rcvd System Get Request");
    //Revisit to see if we need this lock for system get
    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    hal::system_get(request, rsp);
    hal::hal_cfg_db_close();

    return Status::OK;
}

Status
SystemServiceImpl::SystemUUIDGet(ServerContext *context,
                                 const Empty *request,
                                 SystemResponse *rsp)
{
    HAL_TRACE_DEBUG("Rcvd System UUID Get Request");
    hal::system_uuid_get(rsp);
    return Status::OK;
}


Status
SystemServiceImpl::ClearIngressDropStats(ServerContext *context,
                                         const Empty *request,
                                         Empty *rsp) 
{
    HAL_TRACE_DEBUG("Rcvd Clear Ingress Drop Stats");

    hal::clear_ingress_drop_stats();

    return Status::OK;
}

Status
SystemServiceImpl::ClearEgressDropStats(ServerContext *context,
                                        const Empty *request,
                                        Empty *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Clear Egress Drop Stats");

    hal::clear_egress_drop_stats();

    return Status::OK;
}

Status
SystemServiceImpl::ClearPbDropStats(ServerContext *context,
                                    const Empty *request,
                                    Empty *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Clear Pb Drop Stats");

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    hal::hal_cfg_db_close();

    return Status::OK;
}

Status
SystemServiceImpl::ClearFteStats(ServerContext *context,
                                 const Empty *request,
                                 Empty *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Clear FTE Stats");

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    hal::hal_cfg_db_close();

    return Status::OK;
}

Status
SystemServiceImpl::ClearFteTxRxStats(ServerContext *context,
                                     const Empty *request,
                                     Empty *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Clear FTE Tx-Rx Stats");

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    hal::hal_cfg_db_close();

    return Status::OK;
}

Status
SystemServiceImpl::ClearTableStats(ServerContext *context,
                                   const Empty *request,
                                   Empty *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Clear Table Stats");

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    hal::hal_cfg_db_close();

    return Status::OK;
}

Status
SystemServiceImpl::ClearPbStats(ServerContext *context,
                                const Empty *request,
                                Empty *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Clear Pb Stats");

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    hal::clear_pb_stats();
    hal::hal_cfg_db_close();

    return Status::OK;
}

Status
SystemServiceImpl::FeatureProfileGet(ServerContext *context,
                                     const Empty *request,
                                     FeatureProfileResponse *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Get Feature Profile");

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    hal::feature_profile_get(rsp);
    hal::hal_cfg_db_close();

    return Status::OK;
}

Status 
SystemServiceImpl::MicroSegUpdate(ServerContext *context,
                                  const MicroSegUpdateRequestMsg *req,
                                  MicroSegUpdateResponseMsg *rsp)
{
    uint32_t          i, nreqs = req->request_size();
    MicroSegUpdateResponse *response;

    HAL_TRACE_DEBUG("Rcvd Micro Segmentation update");

    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::micro_seg_update(spec, response);
    }
    return Status::OK;
}

Status 
SystemServiceImpl::MicroSegStatusUpdate(ServerContext *context,
                                        const MicroSegRequestMsg *req,
                                        MicroSegResponseMsg *rsp)
{
    uint32_t          i, nreqs = req->request_size();
    MicroSegResponse  *response;

    HAL_TRACE_DEBUG("Rcvd Micro Segmentation status update");

    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::micro_seg_status_update(spec, response);
    }
    return Status::OK;
}

Status 
SystemServiceImpl::SysSpecUpdate(ServerContext* context,
                                 const SysSpec* spec,
                                 SysSpecResponse* rsp)
{
    HAL_TRACE_DEBUG("Rcvd SysSpec update");

    hal::system_handle_fwd_policy_updates(spec, rsp);
    return Status::OK;
}

Status 
SystemServiceImpl::SysSpecGet(ServerContext* context,
                              const SysSpecGetRequest* req,
                              SysSpecGetResponse* rsp)
{
    HAL_TRACE_DEBUG("Rcvd SysSpec get");

    hal::system_get_fwd_policy_mode(rsp);
    return Status::OK;
}
