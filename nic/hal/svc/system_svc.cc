//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// sys service implementation
//------------------------------------------------------------------------------

#include <base.h>
#include <trace.hpp>
#include <hal_cfg.hpp>
#include <system_svc.hpp>
#include <system.pb.h>
#include <system.hpp>

Status
SystemServiceImpl::SystemGet(ServerContext *context,
                             const Empty *request,
                             SystemResponse *rsp)
{
    HAL_TRACE_DEBUG("Rcvd System Get Request");

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    hal::system_get(rsp);

    hal::hal_cfg_db_close(true);
    return Status::OK;
}
