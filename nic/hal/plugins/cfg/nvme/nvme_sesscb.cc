//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nvme_sesscb.hpp"

namespace hal {

void *
nvme_sesscb_get_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((nvme_sesscb_t *)entry)->cb_id);
}

uint32_t
nvme_sesscb_key_size ()
{
    return sizeof(nvme_sesscb_id_t);
}

void *
nvme_sesscb_get_handle_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((nvme_sesscb_t *)entry)->hal_handle);
}

uint32_t
nvme_sesscb_handle_key_size ()
{
    return sizeof(hal_handle_t);
}

static inline hal_ret_t
add_nvme_sesscb_to_db (nvme_sesscb_t *nvme_sesscb)
{
    g_hal_state->nvme_sesscb_id_ht()->insert(nvme_sesscb, &nvme_sesscb->ht_ctxt);
    return HAL_RET_OK;
}


hal_ret_t
nvme_sesscb_create (uint32_t lif,
                    uint32_t g_nsid,
                    uint32_t g_sess_id,
                    uint32_t lif_sess_id,
                    uint32_t ns_sess_id,
                    uint32_t sesq_qid,
                    uint32_t serq_qid,
                    NvmeSessResponse *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    nvme_sesscb_t               *nvme_sesscb;
    pd::pd_nvme_sesscb_create_args_t  pd_nvme_sesscb_args;
    pd::pd_func_args_t          pd_func_args = {0};

    // instantiate NVME SESS CB
    nvme_sesscb = nvme_sesscb_alloc_init();
    if (nvme_sesscb == NULL) {
        return HAL_RET_OOM;
    }

    nvme_sesscb->cb_id = g_sess_id;

    nvme_sesscb->lif = lif;
    nvme_sesscb->g_nsid = g_nsid;
    nvme_sesscb->g_sess_id = g_sess_id;
    nvme_sesscb->lif_sess_id = lif_sess_id;
    nvme_sesscb->ns_sess_id = ns_sess_id;
    nvme_sesscb->sesq_qid = sesq_qid;
    nvme_sesscb->serq_qid = serq_qid;

    nvme_sesscb->hal_handle = hal_alloc_handle();


    // allocate all PD resources and finish programming
    pd::pd_nvme_sesscb_create_args_init(&pd_nvme_sesscb_args);
    pd_nvme_sesscb_args.nvme_sesscb = nvme_sesscb;
    pd_nvme_sesscb_args.rsp = rsp;
    pd_func_args.pd_nvme_sesscb_create = &pd_nvme_sesscb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_NVME_SESSCB_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD NVME SESS CB create failure, err : {}", ret);
        goto cleanup;
    }

cleanup:
    return (ret);
}

} //namspace hal
