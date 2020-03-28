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
#include "nvme_sq_pd.hpp"
#include "nvme_global_pd.hpp"
#include "nvme_pd.hpp"
#include "p4pd_nvme_api.h"
#include "nic/sdk/nvme/nvme_common.h"

namespace hal {
namespace pd {

extern pd_nvme_global_t *g_pd_nvme_global;

void *
nvme_sq_pd_get_hw_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((pd_nvme_sq_t *)entry)->hw_id);
}

uint32_t
nvme_sq_pd_hw_key_size ()
{
    return sizeof(nvme_sq_hw_id_t);
}

/********************************************
 * APIs
 *******************************************/

hal_ret_t
p4pd_add_or_del_nvme_sqcb (pd_nvme_sq_t *nvme_sq_pd, bool del)
{
    hal_ret_t ret = HAL_RET_OK;
    nvme_sqcb_t data = { 0 };
    uint64_t data_addr = 0;
    uint64_t offset;
    
    SDK_ASSERT(nvme_sq_pd != NULL);
    SDK_ASSERT(nvme_sq_pd->nvme_sq != NULL);

    data_addr = lif_manager()->get_lif_qstate_addr(nvme_sq_pd->nvme_sq->lif,
                                                   NVME_QTYPE_SQ, 
                                                   nvme_sq_pd->nvme_sq->sq_id);
    SDK_ASSERT(data_addr != 0);

    //total and host are reversed in auto-generated structs
    data.host = MAX_SQ_RINGS;
    data.total = MAX_SQ_HOST_RINGS;

    get_program_offset((char *)"txdma_stage0.bin",
                       (char *)"nvme_sq_stage0",
                       &offset);
    data.pc = offset; //pc

    data.sq_base_addr = nvme_sq_pd->nvme_sq->base_addr;
    data.log_wqe_size = nvme_sq_pd->nvme_sq->log_wqe_size;
    data.log_num_wqes = nvme_sq_pd->nvme_sq->log_num_wqes;
    data.log_host_page_size = nvme_sq_pd->nvme_sq->log_host_page_size;
    data.cq_id = nvme_sq_pd->nvme_sq->cq_id;
    data.lif_ns_start = nvme_sq_pd->nvme_sq->ns_start;

    HAL_TRACE_DEBUG("sqid: {}, pc: {:#x}", nvme_sq_pd->nvme_sq->sq_id, offset);

    // write to hardware
    HAL_TRACE_DEBUG("LIF: {}: Writing initial SQCB State @{:#x} baseaddr: {:#x} "
                    "sqcb_size: {} log_wqe_size: {} log_num_wqes: {} "
                    "log_host_page_size: {} cq_id: {} ns_start: {}",
                    nvme_sq_pd->nvme_sq->lif, 
                    data_addr, data.sq_base_addr, sizeof(nvme_sqcb_t),
                    data.log_wqe_size, data.log_num_wqes, 
                    data.log_host_page_size, data.cq_id, data.lif_ns_start);

    memrev((uint8_t*)&data, sizeof(data));
    if(!p4plus_hbm_write(data_addr,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write SQCB NVME SQ");
        ret = HAL_RET_HW_FAIL;
    }

    return (ret);
}

hal_ret_t
pd_nvme_sq_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t                ret = HAL_RET_OK;
    pd_nvme_sq_create_args_t *args = pd_func_args->pd_nvme_sq_create;
    pd_nvme_sq_s             *nvme_sq_pd;

    SDK_ASSERT(args != NULL);
    SDK_ASSERT(args->nvme_sq != NULL);

    HAL_TRACE_DEBUG("Creating pd state for NVME SQ.");
    HAL_TRACE_DEBUG("lif: {} sq_id: {} log_wqe_size: {} "
                    "log_num_entries: {} log_host_page_size: {} "
                    "cq_id: {} ns_start: {} base_addr: {}",
                    args->nvme_sq->lif,
                    args->nvme_sq->sq_id,
                    args->nvme_sq->log_wqe_size,
                    args->nvme_sq->log_num_wqes,
                    args->nvme_sq->log_host_page_size,
                    args->nvme_sq->cq_id,
                    args->nvme_sq->ns_start,
                    args->nvme_sq->base_addr);

    // allocate PD nvme_sq state
    nvme_sq_pd = nvme_sq_pd_alloc_init();
    if (nvme_sq_pd == NULL) {
        return HAL_RET_OOM;
    }
    HAL_TRACE_DEBUG("Alloc done");
    nvme_sq_pd->nvme_sq = args->nvme_sq;
    nvme_sq_pd->hw_id = ((uint64_t)args->nvme_sq->lif << 32) | (args->nvme_sq->sq_id);

    HAL_TRACE_DEBUG("Creating NVME NS, PD Handle: {}, PI Handle: {}",
            nvme_sq_pd->hw_id, nvme_sq_pd->nvme_sq->cb_id);

    // program nvme_sq
    ret = p4pd_add_or_del_nvme_sqcb(nvme_sq_pd, false);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    HAL_TRACE_DEBUG("Programming done");

    // add to db
    ret = add_nvme_sq_pd_to_db(nvme_sq_pd);
    if (ret != HAL_RET_OK) {
       goto cleanup;
    }
    HAL_TRACE_DEBUG("DB add done");
    args->nvme_sq->pd = nvme_sq_pd;

    return HAL_RET_OK;

cleanup:

    if (nvme_sq_pd) {
        nvme_sq_pd_free(nvme_sq_pd);
    }
    return ret;
}



} //namespace pd
} //namespace hal
