//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nvme_cq.hpp"

namespace hal {

void *
nvme_cq_get_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((nvme_cq_t *)entry)->cb_id);
}

uint32_t
nvme_cq_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(nvme_cq_id_t)) % ht_size;
}

bool
nvme_cq_compare_key_func (void *key1, void *key2)
{
    SDK_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(nvme_cq_id_t *)key1 == *(nvme_cq_id_t *)key2) {
        return true;
    }
    return false;
}

void *
nvme_cq_get_handle_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((nvme_cq_t *)entry)->hal_handle);
}

uint32_t
nvme_cq_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
nvme_cq_compare_handle_key_func (void *key1, void *key2)
{
    SDK_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

static inline hal_ret_t
add_nvme_cq_to_db (nvme_cq_t *nvme_cq)
{
    g_hal_state->nvme_cq_id_ht()->insert(nvme_cq, &nvme_cq->ht_ctxt);
    return HAL_RET_OK;
}


hal_ret_t
nvme_cq_create (uint32_t lif,
                uint32_t cq_id,
                uint32_t log_wqe_size,
                uint32_t log_num_wqes,
                uint32_t int_num,
                uint64_t base_addr,
                NvmeCqResponse *rsp)
{
    hal_ret_t                    ret = HAL_RET_OK;
    nvme_cq_t                    *nvme_cq;
    pd::pd_nvme_cq_create_args_t pd_nvme_cq_args;
    pd::pd_func_args_t           pd_func_args = {0};

    // instantiate NVME CQ CB
    nvme_cq = nvme_cq_alloc_init();
    if (nvme_cq == NULL) {
        return HAL_RET_OOM;
    }

    nvme_cq->cb_id = ((uint64_t)lif << 32) | (cq_id);

    nvme_cq->lif = lif;
    nvme_cq->cq_id = cq_id;
    nvme_cq->log_wqe_size = log_wqe_size;
    nvme_cq->log_num_wqes = log_num_wqes;
    nvme_cq->int_num = int_num;
    nvme_cq->base_addr = base_addr;

    nvme_cq->hal_handle = hal_alloc_handle();


    // allocate all PD resources and finish programming
    pd::pd_nvme_cq_create_args_init(&pd_nvme_cq_args);
    pd_nvme_cq_args.nvme_cq = nvme_cq;
    pd_nvme_cq_args.rsp = rsp;
    pd_func_args.pd_nvme_cq_create = &pd_nvme_cq_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_NVME_CQ_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD NVME CQ CB create failure, err : {}", ret);
        goto cleanup;
    }

cleanup:
    return (ret);
}


} //namspace hal
