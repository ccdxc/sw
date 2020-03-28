//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "nic/include/base.hpp"
#include <arpa/inet.h>
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/hal.hpp"
#include "nvme_cq_pd.hpp"
#include "nvme_global_pd.hpp"
#include "nvme_pd.hpp"
#include "p4pd_nvme_api.h"
#include "nic/sdk/nvme/nvme_common.h"

namespace hal {
namespace pd {

extern pd_nvme_global_t *g_pd_nvme_global;

void *
nvme_cq_pd_get_hw_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((pd_nvme_cq_t *)entry)->hw_id);
}

uint32_t
nvme_cq_pd_hw_key_size ()
{
    return sizeof(nvme_cq_hw_id_t);
}

uint32_t
nvme_cq_pd_compute_hw_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(nvme_cq_hw_id_t)) % ht_size;
}

/********************************************
 * APIs
 *******************************************/
//64 Bytes
typedef struct nvme_cqcb_s {

    uint8_t  pad2[28];

    uint64_t pad: 51;
    uint64_t cq_full:1;
    uint64_t cq_full_hint:1;
    uint64_t color:1;
    uint64_t log_num_wqes:5;
    uint64_t log_wqe_size:5;

    uint64_t int_assert_addr;
    uint64_t base_addr;

    uint64_t pid : 16;
    uint64_t total : 4;
    uint64_t host : 4;
    uint64_t eval_last : 8;
    uint64_t cos_sel : 8;
    uint64_t cosB : 4;
    uint64_t cosA : 4;
    uint64_t rsvd : 8;
    uint64_t pc : 8;

} PACKED nvme_cqcb_t;


hal_ret_t
p4pd_add_or_del_nvme_cqcb (pd_nvme_cq_t *nvme_cq_pd, bool del, uint64_t *int_addr)
{
    hal_ret_t ret = HAL_RET_OK;
    nvme_cqcb_t data = { 0 };
    uint64_t data_addr = 0;
    
    SDK_ASSERT(nvme_cq_pd != NULL);
    SDK_ASSERT(nvme_cq_pd->nvme_cq != NULL);

    data_addr = lif_manager()->get_lif_qstate_addr(nvme_cq_pd->nvme_cq->lif, 
                                                   NVME_QTYPE_CQ, 
                                                   nvme_cq_pd->nvme_cq->cq_id);
    SDK_ASSERT(data_addr != 0);

    //total and host are reversed in auto-generated structs
    data.host = MAX_CQ_RINGS;
    data.total = MAX_CQ_HOST_RINGS;

    data.base_addr = nvme_cq_pd->nvme_cq->base_addr;
    data.log_wqe_size = nvme_cq_pd->nvme_cq->log_wqe_size;
    data.log_num_wqes = nvme_cq_pd->nvme_cq->log_num_wqes;
    data.int_assert_addr = intr_assert_addr(nvme_cq_pd->nvme_cq->int_num);
    *int_addr = data.int_assert_addr;

    HAL_TRACE_DEBUG("sqid: {}, int_addr: {:#x}", 
                    nvme_cq_pd->nvme_cq->cq_id, *int_addr);

    // write to hardware
    HAL_TRACE_DEBUG("LIF: {}: Writing initial CQCB State @{:#x} baseaddr: {:#x} "
                    "cqcb_size: {} log_wqe_size: {} log_num_wqes: {} "
                    "int_num: {} int_addr: {:#x}", 
                    nvme_cq_pd->nvme_cq->lif, 
                    data_addr, data.base_addr, sizeof(nvme_cqcb_t),
                    data.log_wqe_size, data.log_num_wqes, 
                    nvme_cq_pd->nvme_cq->int_num, data.int_assert_addr);

    memrev((uint8_t*)&data, sizeof(data));
    if(!p4plus_hbm_write(data_addr,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write CQCB NVME CQ");
        ret = HAL_RET_HW_FAIL;
    }

    return (ret);
}

hal_ret_t
pd_nvme_cq_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t                ret = HAL_RET_OK;
    pd_nvme_cq_create_args_t *args = pd_func_args->pd_nvme_cq_create;
    pd_nvme_cq_s             *nvme_cq_pd;
    uint64_t                 int_addr = 0;

    SDK_ASSERT(args != NULL);
    SDK_ASSERT(args->nvme_cq != NULL);

    HAL_TRACE_DEBUG("Creating pd state for NVME CQ.");
    HAL_TRACE_DEBUG("lif: {} cq_id: {} log_wqe_size: {} "
                    "log_num_wqes: {} int_num: {} "
                    "base_addr: {}",
                    args->nvme_cq->lif,
                    args->nvme_cq->cq_id,
                    args->nvme_cq->log_wqe_size,
                    args->nvme_cq->log_num_wqes,
                    args->nvme_cq->int_num,
                    args->nvme_cq->base_addr);

    // allocate PD nvme_cq state
    nvme_cq_pd = nvme_cq_pd_alloc_init();
    if (nvme_cq_pd == NULL) {
        return HAL_RET_OOM;
    }
    HAL_TRACE_DEBUG("Alloc done");
    nvme_cq_pd->nvme_cq = args->nvme_cq;
    nvme_cq_pd->hw_id = ((uint64_t)args->nvme_cq->lif << 32) | (args->nvme_cq->cq_id);

    HAL_TRACE_DEBUG("Creating NVME NS, PD Handle: {}, PI Handle: {}",
            nvme_cq_pd->hw_id, nvme_cq_pd->nvme_cq->cb_id);

    // program nvme_cq
    ret = p4pd_add_or_del_nvme_cqcb(nvme_cq_pd, false, &int_addr);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    HAL_TRACE_DEBUG("Programming done");

    // add to db
    ret = add_nvme_cq_pd_to_db(nvme_cq_pd);
    if (ret != HAL_RET_OK) {
       goto cleanup;
    }
    HAL_TRACE_DEBUG("DB add done");
    args->nvme_cq->pd = nvme_cq_pd;

    if (args->rsp != NULL) {
        args->rsp->set_cq_intr_tbl_addr(int_addr);
    }

    return HAL_RET_OK;

cleanup:

    if (nvme_cq_pd) {
        nvme_cq_pd_free(nvme_cq_pd);
    }
    return ret;
}



} //namespace pd
} //namespace hal
