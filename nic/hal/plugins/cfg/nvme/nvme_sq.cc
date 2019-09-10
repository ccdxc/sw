//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nvme_sq.hpp"

namespace hal {

void *
nvme_sq_get_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((nvme_sq_t *)entry)->cb_id);
}

uint32_t
nvme_sq_key_size ()
{
    return sizeof(nvme_sq_id_t);
}

void *
nvme_sq_get_handle_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((nvme_sq_t *)entry)->hal_handle);
}

uint32_t
nvme_sq_handle_key_size ()
{
    return sizeof(hal_handle_t);
}

static inline hal_ret_t
add_nvme_sq_to_db (nvme_sq_t *nvme_sq)
{
    g_hal_state->nvme_sq_id_ht()->insert(nvme_sq, &nvme_sq->ht_ctxt);
    return HAL_RET_OK;
}


hal_ret_t
nvme_sq_create (uint32_t lif,
                uint32_t sq_id,
                uint32_t log_wqe_size,
                uint32_t log_num_wqes,
                uint32_t log_host_page_size,
                uint32_t cq_id,
                uint32_t ns_start,
                uint64_t base_addr,
                NvmeSqResponse *rsp)
{
    hal_ret_t                    ret = HAL_RET_OK;
    nvme_sq_t                    *nvme_sq;
    pd::pd_nvme_sq_create_args_t pd_nvme_sq_args;
    pd::pd_func_args_t           pd_func_args = {0};

    // instantiate NVME SQ CB
    nvme_sq = nvme_sq_alloc_init();
    if (nvme_sq == NULL) {
        return HAL_RET_OOM;
    }

    nvme_sq->cb_id = ((uint64_t)lif << 32) | (sq_id);

    nvme_sq->lif = lif;
    nvme_sq->sq_id = sq_id;
    nvme_sq->log_wqe_size = log_wqe_size;
    nvme_sq->log_num_wqes = log_num_wqes;
    nvme_sq->log_host_page_size = log_host_page_size;
    nvme_sq->cq_id = cq_id;
    nvme_sq->ns_start = ns_start;
    nvme_sq->base_addr = base_addr;

    nvme_sq->hal_handle = hal_alloc_handle();


    // allocate all PD resources and finish programming
    pd::pd_nvme_sq_create_args_init(&pd_nvme_sq_args);
    pd_nvme_sq_args.nvme_sq = nvme_sq;
    pd_nvme_sq_args.rsp = rsp;
    pd_func_args.pd_nvme_sq_create = &pd_nvme_sq_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_NVME_SQ_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD NVME SQ CB create failure, err : {}", ret);
        goto cleanup;
    }

cleanup:
    return (ret);
}


} //namspace hal
