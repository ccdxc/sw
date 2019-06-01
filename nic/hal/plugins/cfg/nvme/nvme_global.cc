//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nvme_global.hpp"

namespace hal {

hal_ret_t 
nvme_global_create (uint32_t max_lif, 
                    uint32_t max_ns, 
                    uint32_t max_sess,
                    uint32_t max_cmd_context, 
                    uint32_t tx_max_pdu_context,
                    uint32_t rx_max_pdu_context)
{
    hal_ret_t                   ret = HAL_RET_OK;
    nvme_global_t               *nvme_global;
    pd::pd_nvme_global_create_args_t  pd_nvme_global_args;
    pd::pd_func_args_t          pd_func_args = {0};

    // instantiate NVME SESS CB
    nvme_global = nvme_global_alloc_init();
    if (nvme_global == NULL) {
        return HAL_RET_OOM;
    }

    nvme_global->max_lif = max_lif;
    nvme_global->max_ns = max_ns;
    nvme_global->max_sess = max_sess;
    nvme_global->max_cmd_context = max_cmd_context;
    nvme_global->tx_max_pdu_context = tx_max_pdu_context;
    nvme_global->rx_max_pdu_context = rx_max_pdu_context;

    nvme_global->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming
    pd::pd_nvme_global_create_args_init(&pd_nvme_global_args);
    pd_nvme_global_args.nvme_global = nvme_global;
    pd_func_args.pd_nvme_global_create = &pd_nvme_global_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_NVME_GLOBAL_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD NVME SESS CB create failure, err : {}", ret);
        goto cleanup;
    }

cleanup:
    return (ret);
}



} //namspace hal
