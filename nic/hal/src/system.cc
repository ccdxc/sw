// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/system.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {

//------------------------------------------------------------------------------
// process a vrf get request
//------------------------------------------------------------------------------
hal_ret_t
system_get(SystemResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd::pd_system_args_t    pd_system_args = { 0 };
    pd::pd_drop_stats_get_args_t d_args;
    pd::pd_table_stats_get_args_t t_args;
    
    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("Querying Drop Stats:");


    pd::pd_system_args_init(&pd_system_args);
    pd_system_args.rsp = rsp;


    d_args.pd_sys_args = &pd_system_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_DROP_STATS_GET, (void *)&d_args);
    // ret = pd::pd_drop_stats_get(&pd_system_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get drop stats, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

    t_args.pd_sys_args = &pd_system_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_TABLE_STATS_GET, (void *)&t_args);
    // ret = pd::pd_table_stats_get(&pd_system_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get drop stats, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

    rsp->set_api_status(types::API_STATUS_OK);

end:
    return HAL_RET_OK;
}

hal_ret_t
system_set(const SystemConfigMsg *req)
{
    hal::pd::pd_system_drop_action_args_t args;
    uint32_t nreqs = req->request_size();
    if (nreqs != 1) {
        HAL_TRACE_ERR("got more than one entry in request");
        return HAL_RET_INVALID_ARG;
    }
    auto spec = req->request(0);
    args.spec = &spec;
    // return pd::hal_pd_call(pd::pd_system_drop_action(&args);
    return pd::hal_pd_call(pd::PD_FUNC_ID_SYSTEM_DROP_ACTION, (void*)&args);
}

}    // namespace hal
