// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <base.h>
#include <hal.hpp>
#include <hal_lock.hpp>
#include <hal_state.hpp>
#include <system.hpp>
#include <pd_api.hpp>

namespace hal {

//------------------------------------------------------------------------------
// process a tenant get request
//------------------------------------------------------------------------------
hal_ret_t
system_get(SystemResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd::pd_system_args_t    pd_system_args = { 0 };
    
    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("Querying Drop Stats:");


    pd::pd_system_args_init(&pd_system_args);
    pd_system_args.rsp = rsp;


    ret = pd::pd_drop_stats_get(&pd_system_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get drop stats, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

    rsp->set_api_status(types::API_STATUS_OK);

end:
    return HAL_RET_OK;
}

}    // namespace hal
