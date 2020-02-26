//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/src/internal/cpucb.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"
#include "nic/hal/plugins/cfg/lif/lif.hpp"

namespace hal {
void *
cpucb_get_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((cpucb_t *)entry)->cb_id);
}

uint32_t
cpucb_key_size ()
{
    return sizeof(cpucb_id_t);
}

void *
cpucb_get_handle_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((cpucb_t *)entry)->hal_handle);
}

uint32_t
cpucb_handle_key_size ()
{
    return sizeof(hal_handle_t);
}

#ifdef __x86_64__
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
#endif

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

#ifdef __x86_64__
static inline void
cpucb_spec_to_cb(CpuCbSpec& spec, cpucb_t& cpucb)
{
    cpucb.cb_id = spec.key_or_handle().cpucb_id();
    cpucb.debug_dol = spec.debug_dol();
    cpucb_set_default_params(cpucb);
}
#endif

hal_ret_t
cpucb_get_by_id(cpucb_id_t cpucb_id, cpucb_t &cpucb, lif_id_t lif_id)
{
    hal_ret_t ret = HAL_RET_OK;

    pd::pd_cpucb_get_args_t pd_cpucb_args;
    pd::pd_func_args_t      pd_func_args = {0};

    cpucb_init(&cpucb);
    cpucb.cb_id = cpucb_id;
    pd::pd_cpucb_get_args_init(&pd_cpucb_args);
    pd_cpucb_args.cpucb = &cpucb;

    pd_func_args.pd_cpucb_get = &pd_cpucb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_CPUCB_GET, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD CPUCB: Failed to get, err: {}", ret);
        return HAL_RET_HW_FAIL;
    }
    return HAL_RET_OK;
}

#ifdef __x86_64__
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
    pd::pd_func_args_t          pd_func_args = {0};

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
    pd_func_args.pd_cpucb_create = &pd_cpucb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_CPUCB_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD CPU CB create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // add this L2 segment to our db
    ret = add_cpucb_to_db(cpucb);
    SDK_ASSERT(ret == HAL_RET_OK);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_cpucb_status()->set_cpucb_handle(cpucb->hal_handle);
    return HAL_RET_OK;

cleanup:

    cpucb_free(cpucb);
    return ret;
}
#endif

//------------------------------------------------------------------------------
// create a CPU CB with cpucb_id
// TODO: if CPU CB exists, treat this as modify (vrf id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
cpucb_create (uint8_t cpucb_id)
{
    hal_ret_t                   ret = HAL_RET_OK;
    cpucb_t                     *cpucb;
    pd::pd_cpucb_create_args_t  pd_cpucb_args;
    pd::pd_func_args_t          pd_func_args = {0};

    cpucb = cpucb_alloc_init();
    if (cpucb == NULL) {
        return HAL_RET_OOM;
    }

    cpucb->cb_id = cpucb_id;
    cpucb->debug_dol = 0;
    cpucb_set_default_params(*cpucb);
    cpucb->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming
    pd::pd_cpucb_create_args_init(&pd_cpucb_args);
    pd_cpucb_args.cpucb = cpucb;
    pd_func_args.pd_cpucb_create = &pd_cpucb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_CPUCB_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD CPU CB create failure, err : {}", ret);
        goto cleanup;
    }

    // add this L2 segment to our db
    ret = add_cpucb_to_db(cpucb);
    SDK_ASSERT(ret == HAL_RET_OK);

    return HAL_RET_OK;

cleanup:

    cpucb_free(cpucb);
    return ret;
}

#ifdef __x86_64__
//------------------------------------------------------------------------------
// process a CPU CB update request
//------------------------------------------------------------------------------
hal_ret_t
cpucb_update (CpuCbSpec& spec, CpuCbResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    cpucb_t*               cpucb;
    pd::pd_cpucb_update_args_t    pd_cpucb_args;
    pd::pd_func_args_t          pd_func_args = {0};

    auto kh = spec.key_or_handle();

    cpucb = find_cpucb_by_id(kh.cpucb_id());
    if (cpucb == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_CPU_CB_NOT_FOUND;
    }

    pd::pd_cpucb_update_args_init(&pd_cpucb_args);
    pd_cpucb_args.cpucb = cpucb;

    cpucb_spec_to_cb(spec, *cpucb);

    pd_func_args.pd_cpucb_update = &pd_cpucb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_CPUCB_UPDATE, &pd_func_args);
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
    CpuCbGetResponse *rsp = resp->add_response();

    auto kh = req.key_or_handle();

    cpucb = find_cpucb_by_id(kh.cpucb_id());
    if (cpucb == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_CPU_CB_NOT_FOUND;
    }
    // When we move to multiple CPU lif, pass the lif
    ret = cpucb_get_by_id(cpucb->cb_id, rcpucb);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("cpucb id: {} not found in pd", cpucb->cb_id);
        return HAL_RET_CPU_CB_NOT_FOUND;
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
    pd::pd_func_args_t          pd_func_args = {0};

    auto kh = req.key_or_handle();
    cpucb = find_cpucb_by_id(kh.cpucb_id());
    if (cpucb == NULL) {
        rsp->set_api_status(types::API_STATUS_OK);
        return HAL_RET_OK;
    }

    pd::pd_cpucb_delete_args_init(&pd_cpucb_args);
    pd_cpucb_args.cpucb = cpucb;

    pd_func_args.pd_cpucb_delete = &pd_cpucb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_CPUCB_DELETE, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD CPUCB: delete Failed, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill stats of this CPU CB
    rsp->set_api_status(types::API_STATUS_OK);

    return HAL_RET_OK;
}

#endif

hal_ret_t
cpucb_get_stats ( lif_id_t lif_id, LifGetResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    cpucb_t   cpucb;
    intf::CpuLifStats *cpu_stats = rsp->mutable_stats()->mutable_cpu_lif_stats();
    for(int i = 0; i < types::CpucbId_ARRAYSIZE; i++) {
        ret = cpucb_get_by_id( i, cpucb, lif_id);
        if (ret == HAL_RET_OK) {
            intf::CpuQueueStats *qstats = cpu_stats->add_cpu_queue_stats();
            qstats->set_cpucb_id((types::CpucbId)i);
            qstats->set_num_tx_packets(cpucb.total_tx_pkts);
            qstats->set_num_rx_packets(cpucb.total_rx_pkts);
            qstats->set_rx_qfull_drop_errors(cpucb.rx_qfull_drop_errors);
            qstats->set_tx_sem_full_drops(cpucb.tx_sem_full_drops);
            qstats->set_tx_free_requests(cpucb.ascq_free_requests);
            qstats->set_rx_sem_full_drops(cpucb.rx_sem_full_drops);
            qstats->set_rx_queue0_pkts(cpucb.rx_queue0_pkts);
            qstats->set_rx_queue1_pkts(cpucb.rx_queue1_pkts);
            qstats->set_rx_queue2_pkts(cpucb.rx_queue2_pkts);
        }
    }
    return ret;
}

}    // namespace hal
