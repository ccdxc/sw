//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/internal/cpucb.hpp"
#include "nic/hal/src/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"

namespace hal {
void *
cpucb_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((cpucb_t *)entry)->cb_id);
}

uint32_t
cpucb_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(cpucb_id_t)) % ht_size;
}

bool
cpucb_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(cpucb_id_t *)key1 == *(cpucb_id_t *)key2) {
        return true;
    }
    return false;
}

void *
cpucb_get_handle_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((cpucb_t *)entry)->hal_handle);
}

uint32_t
cpucb_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
cpucb_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// validate an incoming CPUCB create request
// TODO:
// 1. check if CPUCB exists already
//------------------------------------------------------------------------------
static hal_ret_t
validate_cpucb_create (CpuCbSpec& spec, CpuCbResponse *rsp)
{
    // must have key-handle set
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_CPU_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    // must have key in the key-handle
    if (spec.key_or_handle().key_or_handle_case() !=
            CpuCbKeyHandle::kCpucbId) {
        rsp->set_api_status(types::API_STATUS_CPU_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert this CPU CB in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_cpucb_to_db (cpucb_t *cpucb)
{
    g_hal_state->cpucb_id_ht()->insert(cpucb, &cpucb->ht_ctxt);
    return HAL_RET_OK;
}

static inline void
cpucb_set_default_params(cpucb_t& cpucb)
{
    switch(cpucb.cb_id) {
    case types::CPUCB_ID_QUIESCE:
        cpucb.cfg_flags |= CPUCB_FLAG_ADD_QS_PKT_TRLR;
    }
}
static inline void
cpucb_spec_to_cb(CpuCbSpec& spec, cpucb_t& cpucb)
{
    cpucb.cb_id = spec.key_or_handle().cpucb_id();
    cpucb.debug_dol = spec.debug_dol();
    cpucb_set_default_params(cpucb);
}
//------------------------------------------------------------------------------
// process a CPU CB create request
// TODO: if CPU CB exists, treat this as modify (vrf id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
cpucb_create (CpuCbSpec& spec, CpuCbResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    cpucb_t                *cpucb;
    pd::pd_cpucb_create_args_t    pd_cpucb_args;

    // validate the request message
    ret = validate_cpucb_create(spec, rsp);

    cpucb = cpucb_alloc_init();
    if (cpucb == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }

    cpucb_spec_to_cb(spec, *cpucb);

    cpucb->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming
    pd::pd_cpucb_create_args_init(&pd_cpucb_args);
    pd_cpucb_args.cpucb = cpucb;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_CPUCB_CREATE, (void *)&pd_cpucb_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD CPU CB create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // add this L2 segment to our db
    ret = add_cpucb_to_db(cpucb);
    HAL_ASSERT(ret == HAL_RET_OK);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_cpucb_status()->set_cpucb_handle(cpucb->hal_handle);
    return HAL_RET_OK;

cleanup:

    cpucb_free(cpucb);
    return ret;
}

//------------------------------------------------------------------------------
// process a CPU CB update request
//------------------------------------------------------------------------------
hal_ret_t
cpucb_update (CpuCbSpec& spec, CpuCbResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    cpucb_t*               cpucb;
    pd::pd_cpucb_update_args_t    pd_cpucb_args;

    auto kh = spec.key_or_handle();

    cpucb = find_cpucb_by_id(kh.cpucb_id());
    if (cpucb == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_CPU_CB_NOT_FOUND;
    }

    pd::pd_cpucb_update_args_init(&pd_cpucb_args);
    pd_cpucb_args.cpucb = cpucb;

    cpucb_spec_to_cb(spec, *cpucb);

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_CPUCB_UPDATE, (void *)&pd_cpucb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD CPUCB: Update Failed, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    rsp->set_api_status(types::API_STATUS_OK);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a CPU CB get request
//------------------------------------------------------------------------------
hal_ret_t
cpucb_get (CpuCbGetRequest& req, CpuCbGetResponseMsg *resp)
{
    hal_ret_t              ret = HAL_RET_OK;
    cpucb_t                rcpucb;
    cpucb_t*               cpucb;
    pd::pd_cpucb_get_args_t    pd_cpucb_args;
    CpuCbGetResponse *rsp = resp->add_response();

    auto kh = req.key_or_handle();

    cpucb = find_cpucb_by_id(kh.cpucb_id());
    if (cpucb == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_CPU_CB_NOT_FOUND;
    }

    cpucb_init(&rcpucb);
    rcpucb.cb_id = cpucb->cb_id;
    pd::pd_cpucb_get_args_init(&pd_cpucb_args);
    pd_cpucb_args.cpucb = &rcpucb;

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_CPUCB_GET, (void *)&pd_cpucb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD CPUCB: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill config spec of this CPU CB
    rsp->mutable_spec()->mutable_key_or_handle()->set_cpucb_id(rcpucb.cb_id);

    // fill operational state of this CPU CB
    rsp->mutable_status()->set_cpucb_handle(cpucb->hal_handle);

    // fill stats of this CPU CB
    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a CPU CB delete request
//------------------------------------------------------------------------------
hal_ret_t
cpucb_delete (cpucb::CpuCbDeleteRequest& req, cpucb::CpuCbDeleteResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    cpucb_t*               cpucb;
    pd::pd_cpucb_delete_args_t    pd_cpucb_args;

    auto kh = req.key_or_handle();
    cpucb = find_cpucb_by_id(kh.cpucb_id());
    if (cpucb == NULL) {
        rsp->set_api_status(types::API_STATUS_OK);
        return HAL_RET_OK;
    }

    pd::pd_cpucb_delete_args_init(&pd_cpucb_args);
    pd_cpucb_args.cpucb = cpucb;

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_CPUCB_DELETE, (void *)&pd_cpucb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD CPUCB: delete Failed, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill stats of this CPU CB
    rsp->set_api_status(types::API_STATUS_OK);

    return HAL_RET_OK;
}

}    // namespace hal
