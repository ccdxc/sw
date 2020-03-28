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
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/hal.hpp"
#include "nvme_ns_pd.hpp"
#include "nvme_global_pd.hpp"
#include "nvme_pd.hpp"
#include "p4pd_nvme_api.h"
#include "nic/sdk/nvme/nvme_common.h"

namespace hal {
namespace pd {

extern pd_nvme_global_t *g_pd_nvme_global;

void *
nvme_ns_pd_get_hw_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((pd_nvme_ns_t *)entry)->hw_id);
}

uint32_t
nvme_ns_pd_hw_key_size ()
{
    return sizeof(nvme_ns_hw_id_t);
}

/********************************************
 * APIs
 *******************************************/

hal_ret_t
p4pd_add_or_del_nvme_nscb (pd_nvme_ns_t *nvme_ns_pd, bool del, uint64_t *nscb_addr)
{
    hal_ret_t ret = HAL_RET_OK;
    nvme_nscb_t data = { 0 };
    uint64_t data_addr = 0;
    uint32_t g_nsid;
    
    SDK_ASSERT(nvme_ns_pd != NULL);
    SDK_ASSERT(nvme_ns_pd->nvme_ns != NULL);

    g_nsid = nvme_ns_pd->nvme_ns->g_nsid;

    data_addr = g_pd_nvme_global->nscb_base_addr + g_nsid * sizeof(nvme_nscb_t);
    SDK_ASSERT(data_addr != 0);
    *nscb_addr = data_addr;

    data.backend_ns_id = nvme_ns_pd->nvme_ns->backend_nsid;
    data.ns_size = nvme_ns_pd->nvme_ns->size;
    data.log_lba_size = log2(nvme_ns_pd->nvme_ns->lba_size);
    data.key_index = nvme_ns_pd->nvme_ns->key_index;
    data.sec_key_index = nvme_ns_pd->nvme_ns->sec_key_index;
    data.sess_prodcb_start = nvme_ns_pd->nvme_ns->g_sess_start;

    HAL_TRACE_DEBUG("Programming nscb at addr: {:#x}", 
                    data_addr);

    memrev((uint8_t*)&data, sizeof(data));
    if(!p4plus_hbm_write(data_addr,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write nscb for NVME NSCB");
        ret = HAL_RET_HW_FAIL;
    }

    return (ret);
}

hal_ret_t
pd_nvme_ns_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t                ret = HAL_RET_OK;
    pd_nvme_ns_create_args_t *args = pd_func_args->pd_nvme_ns_create;
    pd_nvme_ns_s             *nvme_ns_pd;
    uint64_t                 nscb_addr = 0;

    SDK_ASSERT(args != NULL);
    SDK_ASSERT(args->nvme_ns != NULL);

    HAL_TRACE_DEBUG("Creating pd state for NVME NS.");
    HAL_TRACE_DEBUG("lif: {} lif_nsid: {} g_nsid: {} backend_nsid: {} "
                    "max_sessions: {} size: {} lba_size: {} "
                    "key_index: {} sec_key_index: {}"
                    "g_sess_start: {}",
                    args->nvme_ns->lif,
                    args->nvme_ns->lif_nsid,
                    args->nvme_ns->g_nsid,
                    args->nvme_ns->backend_nsid,
                    args->nvme_ns->max_sessions,
                    args->nvme_ns->size,
                    args->nvme_ns->lba_size,
                    args->nvme_ns->key_index,
                    args->nvme_ns->sec_key_index,
                    args->nvme_ns->g_sess_start);

    // allocate PD nvme_ns state
    nvme_ns_pd = nvme_ns_pd_alloc_init();
    if (nvme_ns_pd == NULL) {
        return HAL_RET_OOM;
    }
    HAL_TRACE_DEBUG("Alloc done");
    nvme_ns_pd->nvme_ns = args->nvme_ns;
    nvme_ns_pd->hw_id = args->nvme_ns->g_nsid;

    HAL_TRACE_DEBUG("Creating NVME NS, PD Handle: {}, PI Handle: {}",
            nvme_ns_pd->hw_id, nvme_ns_pd->nvme_ns->cb_id);

    // program nvme_ns
    ret = p4pd_add_or_del_nvme_nscb(nvme_ns_pd, false, &nscb_addr);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    HAL_TRACE_DEBUG("Programming done");

    // add to db
    ret = add_nvme_ns_pd_to_db(nvme_ns_pd);
    if (ret != HAL_RET_OK) {
       goto cleanup;
    }
    HAL_TRACE_DEBUG("DB add done");
    args->nvme_ns->pd = nvme_ns_pd;

    if (args->rsp != NULL) {
        args->rsp->set_nscb_addr(nscb_addr);
    }

    return HAL_RET_OK;

cleanup:

    if (nvme_ns_pd) {
        nvme_ns_pd_free(nvme_ns_pd);
    }
    return ret;
}


/*
 * Invoked by sess_create() when a new session is added for this
 * name space
 * Idea is to read the nscb, modify the bitmap and write back
 * XXX: Need to see if there are any race conditions
 */
hal_ret_t
nvme_ns_update_session_id (uint32_t g_nsid, uint16_t sess_id)
{
    hal_ret_t      ret = HAL_RET_OK;
    uint64_t       addr;
    nvme_nscb_t    data = { 0 };
    uint8_t        bit_index, byte_index;

    SDK_ASSERT(sess_id < 256); //XXX
    //Read NSCB state and verify against max_sessions

    addr = g_pd_nvme_global->nscb_base_addr + g_nsid * sizeof(nvme_nscb_t);

    if(sdk::asic::asic_mem_read(addr,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to read NVME NSCB for g_nsid: {}",
                      g_nsid);
        return HAL_RET_HW_FAIL;
    }

    // Convert data after reading from HBM
    memrev((uint8_t*)&data, sizeof(data));

    byte_index = sess_id / 8; //Byte number
    bit_index  = sess_id % 8; //Bit in Byte
    data.valid_session_bitmap[byte_index] |= (1 << bit_index);

    memrev((uint8_t*)&data, sizeof(data));
    if(!p4plus_hbm_write(addr,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write NVME NSCB for g_nsid: {}",
                      g_nsid);
        ret = HAL_RET_HW_FAIL;
    }

    return (ret);
}


} //namespace pd
} //namespace hal
