//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nvme_ns.hpp"

namespace hal {

void *
nvme_ns_get_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((nvme_ns_t *)entry)->cb_id);
}

uint32_t
nvme_ns_key_size ()
{
    return sizeof(nvme_ns_id_t);
}

void *
nvme_ns_get_handle_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((nvme_ns_t *)entry)->hal_handle);
}

uint32_t
nvme_ns_handle_key_size ()
{
    return sizeof(hal_handle_t);
}

static inline hal_ret_t
add_nvme_ns_to_db (nvme_ns_t *nvme_ns)
{
    g_hal_state->nvme_ns_id_ht()->insert(nvme_ns, &nvme_ns->ht_ctxt);
    return HAL_RET_OK;
}


hal_ret_t
nvme_ns_create (uint32_t lif,
                  uint32_t lif_nsid,
                  uint32_t g_nsid,
                  uint32_t backend_nsid,
                  uint32_t max_sessions,
                  uint32_t size, //in LBAs
                  uint32_t lba_size,
                  uint32_t key_index,
                  uint32_t sec_key_index, 
                  uint32_t g_sess_start,
                  NvmeNsResponse *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    nvme_ns_t                 *nvme_ns;
    pd::pd_nvme_ns_create_args_t  pd_nvme_ns_args;
    pd::pd_func_args_t          pd_func_args = {0};

    // instantiate NVME NS CB
    nvme_ns = nvme_ns_alloc_init();
    if (nvme_ns == NULL) {
        return HAL_RET_OOM;
    }

    nvme_ns->cb_id = g_nsid;

    nvme_ns->lif = lif;
    nvme_ns->g_nsid = g_nsid;
    nvme_ns->lif_nsid = lif_nsid;
    nvme_ns->backend_nsid = backend_nsid;
    nvme_ns->max_sessions = max_sessions;
    nvme_ns->size = size;
    nvme_ns->lba_size = lba_size;
    nvme_ns->g_sess_start = g_sess_start;
    nvme_ns->key_index = key_index;
    nvme_ns->sec_key_index = sec_key_index;

    nvme_ns->hal_handle = hal_alloc_handle();


    // allocate all PD resources and finish programming
    pd::pd_nvme_ns_create_args_init(&pd_nvme_ns_args);
    pd_nvme_ns_args.nvme_ns = nvme_ns;
    pd_nvme_ns_args.rsp = rsp;
    pd_func_args.pd_nvme_ns_create = &pd_nvme_ns_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_NVME_NS_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD NVME NS CB create failure, err : {}", ret);
        goto cleanup;
    }

cleanup:
    return (ret);
}

} //namspace hal
