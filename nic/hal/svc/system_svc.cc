//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// sys service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/hal/svc/system_svc.hpp"
#include "nic/gen/proto/hal/system.pb.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/hal/src/internal/system.hpp"

Status
SystemServiceImpl::ApiStatsGet(ServerContext *context,
                               const Empty *request,
                               ApiStatsResponse *rsp)
{
    HAL_TRACE_DEBUG("Rcvd API Stats Get Request");

    hal::api_stats_get(rsp);

    return Status::OK;
}

Status
SystemServiceImpl::SystemGet(ServerContext *context,
                             const Empty *request,
                             SystemResponse *rsp)
{
    HAL_TRACE_DEBUG("Rcvd System Get Request");

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    hal::system_get(rsp);

    hal::hal_cfg_db_close();
    return Status::OK;
}


Status
SystemServiceImpl::SystemConfig(ServerContext *context,
                             const  SystemConfigMsg *request,
                             SystemConfigResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd System Config Request");
    hal_ret_t ret;
    ret = hal::system_set(request);
    rsp->add_response();
    if (ret != HAL_RET_OK) {
        rsp->set_status(types::API_STATUS_HW_WRITE_ERROR);
        return Status::OK;
    }
    return Status::OK;
}
