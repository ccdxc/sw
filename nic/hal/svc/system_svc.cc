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
#include "nic/hal/src/system.hpp"

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
