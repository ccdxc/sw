//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/src/internal/rawrcb.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/app_redir_shared.h"

namespace hal {
void *
rawrcb_get_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((rawrcb_t *)entry)->cb_id);
}

uint32_t
rawrcb_key_size ()
{
    return sizeof(rawrcb_id_t);
}

void *
rawrcb_get_handle_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((rawrcb_t *)entry)->hal_handle);
}

uint32_t
rawrcb_handle_key_size (void *key, uint32_t ht_size)
{
    return sizeof(hal_handle_t);
}

// allocate a RAWRCB instance
static inline rawrcb_t *
rawrcb_alloc (void)
{
    rawrcb_t    *rawrcb;

    rawrcb = (rawrcb_t *)g_hal_state->rawrcb_slab()->alloc();
    if (rawrcb == NULL) {
        return NULL;
    }
    return rawrcb;
}

// initialize a RAWRCB instance
static inline rawrcb_t *
rawrcb_init (rawrcb_t *rawrcb)
{
    if (!rawrcb) {
        return NULL;
    }
    SDK_SPINLOCK_INIT(&rawrcb->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    rawrcb->pd = NULL;

    // initialize meta information
    rawrcb->ht_ctxt.reset();
    rawrcb->hal_handle_ht_ctxt.reset();

    return rawrcb;
}

// allocate and initialize a RAWRCB instance
static inline rawrcb_t *
rawrcb_alloc_init (void)
{
    return rawrcb_init(rawrcb_alloc());
}

static inline hal_ret_t
rawrcb_free (rawrcb_t *rawrcb)
{
    SDK_SPINLOCK_DESTROY(&rawrcb->slock);
    hal::delay_delete_to_slab(HAL_SLAB_RAWRCB, rawrcb);
    return HAL_RET_OK;
}

rawrcb_t *
find_rawrcb_by_id (rawrcb_id_t rawrcb_id)
{
    return (rawrcb_t *)g_hal_state->rawrcb_id_ht()->lookup(&rawrcb_id);
}

//------------------------------------------------------------------------------
// validate an incoming RAWRCB create request
// TODO:
// 1. check if RAWRCB exists already
//------------------------------------------------------------------------------
static hal_ret_t
validate_rawrcb_create (RawrCbSpec& spec, RawrCbResponse *rsp)
{
    // must have key-handle set
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_RAWR_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    // must have key in the key-handle
    if (spec.key_or_handle().key_or_handle_case() !=
            RawrCbKeyHandle::kRawrcbId) {
        rsp->set_api_status(types::API_STATUS_RAWR_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    if (spec.key_or_handle().rawrcb_id() >= RAWRCB_NUM_ENTRIES_MAX) {
        rsp->set_api_status(types::API_STATUS_RAWR_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert this RAWR CB in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_rawrcb_to_db (rawrcb_t *rawrcb)
{
    g_hal_state->rawrcb_id_ht()->insert(rawrcb, &rawrcb->ht_ctxt);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a RAWR CB create request
// TODO: if RAWR CB exists, treat this as modify (vrf id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
rawrcb_create (RawrCbSpec& spec, RawrCbResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;
    rawrcb_t                *rawrcb = NULL;
    pd::pd_rawrcb_create_args_t    pd_rawrcb_args;
    pd::pd_func_args_t          pd_func_args = {0};

    // validate the request message
    ret = validate_rawrcb_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        goto cleanup;
    }

    // instantiate RAWR CB
    rawrcb = rawrcb_alloc_init();
    if (rawrcb == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
        goto cleanup;
    }

    rawrcb->cb_id = spec.key_or_handle().rawrcb_id();
    rawrcb->rawrcb_flags = spec.rawrcb_flags();
    rawrcb->chain_rxq_base = spec.chain_rxq_base();
    rawrcb->chain_rxq_ring_indices_addr = spec.chain_rxq_ring_indices_addr();
    rawrcb->chain_rxq_ring_size_shift = spec.chain_rxq_ring_size_shift();
    rawrcb->chain_rxq_entry_size_shift = spec.chain_rxq_entry_size_shift();
    rawrcb->chain_rxq_ring_index_select = spec.chain_rxq_ring_index_select();

    rawrcb->chain_txq_base = spec.chain_txq_base();
    rawrcb->chain_txq_ring_indices_addr = spec.chain_txq_ring_indices_addr();
    rawrcb->chain_txq_ring_size_shift = spec.chain_txq_ring_size_shift();
    rawrcb->chain_txq_entry_size_shift = spec.chain_txq_entry_size_shift();
    rawrcb->chain_txq_ring_index_select = spec.chain_txq_ring_index_select();
    rawrcb->chain_txq_lif = spec.chain_txq_lif();
    rawrcb->chain_txq_qtype = spec.chain_txq_qtype();
    rawrcb->chain_txq_qid = spec.chain_txq_qid();
    rawrcb->cpu_id = spec.cpu_id();
    rawrcb->ascq_base = spec.ascq_base();
    rawrcb->ascq_sem_inf_addr = spec.ascq_sem_inf_addr();

    rawrcb->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming
    pd::pd_rawrcb_create_args_init(&pd_rawrcb_args);
    pd_rawrcb_args.rawrcb = rawrcb;
    pd_func_args.pd_rawrcb_create = &pd_rawrcb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_RAWRCB_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD RAWR CB create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // add this RAWR CB to our db
    ret = add_rawrcb_to_db(rawrcb);
    SDK_ASSERT(ret == HAL_RET_OK);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_rawrcb_status()->set_rawrcb_handle(rawrcb->hal_handle);
    return HAL_RET_OK;

cleanup:

    if (rawrcb) {
        rawrcb_free(rawrcb);
    }
    return ret;
}

//------------------------------------------------------------------------------
// process a RAWR CB update request
//------------------------------------------------------------------------------
hal_ret_t
rawrcb_update (RawrCbSpec& spec, RawrCbResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;
    rawrcb_t*               rawrcb;
    pd::pd_rawrcb_update_args_t    pd_rawrcb_args;
    pd::pd_func_args_t          pd_func_args = {0};

    auto kh = spec.key_or_handle();

    rawrcb = find_rawrcb_by_id(kh.rawrcb_id());
    if (rawrcb == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_RAWR_CB_NOT_FOUND;
    }

    rawrcb->rawrcb_flags = spec.rawrcb_flags();
    rawrcb->chain_rxq_base = spec.chain_rxq_base();
    rawrcb->chain_rxq_ring_indices_addr = spec.chain_rxq_ring_indices_addr();
    rawrcb->chain_rxq_ring_size_shift = spec.chain_rxq_ring_size_shift();
    rawrcb->chain_rxq_entry_size_shift = spec.chain_rxq_entry_size_shift();
    rawrcb->chain_rxq_ring_index_select = spec.chain_rxq_ring_index_select();

    rawrcb->chain_txq_base = spec.chain_txq_base();
    rawrcb->chain_txq_ring_indices_addr = spec.chain_txq_ring_indices_addr();
    rawrcb->chain_txq_ring_size_shift = spec.chain_txq_ring_size_shift();
    rawrcb->chain_txq_entry_size_shift = spec.chain_txq_entry_size_shift();
    rawrcb->chain_txq_ring_index_select = spec.chain_txq_ring_index_select();
    rawrcb->chain_txq_lif = spec.chain_txq_lif();
    rawrcb->chain_txq_qtype = spec.chain_txq_qtype();
    rawrcb->chain_txq_qid = spec.chain_txq_qid();
    rawrcb->cpu_id = spec.cpu_id();
    rawrcb->ascq_base = spec.ascq_base();
    rawrcb->ascq_sem_inf_addr = spec.ascq_sem_inf_addr();

    pd::pd_rawrcb_update_args_init(&pd_rawrcb_args);
    pd_rawrcb_args.rawrcb = rawrcb;

    pd_func_args.pd_rawrcb_update = &pd_rawrcb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_RAWRCB_UPDATE, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD RAWRCB: Update Failed, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill stats of this RAWR CB
    rsp->set_api_status(types::API_STATUS_OK);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a RAWR CB get request
//------------------------------------------------------------------------------
hal_ret_t
rawrcb_get (RawrCbGetRequest& req, RawrCbGetResponseMsg *rsp_msg)
{
    hal_ret_t               ret = HAL_RET_OK;
    rawrcb_t                rrawrcb;
    rawrcb_t*               rawrcb;
    pd::pd_rawrcb_get_args_t    pd_rawrcb_args;
    pd::pd_func_args_t          pd_func_args = {0};
    RawrCbGetResponse *rsp = rsp_msg->add_response();

    auto kh = req.key_or_handle();

    rawrcb = find_rawrcb_by_id(kh.rawrcb_id());
    if (rawrcb == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_RAWR_CB_NOT_FOUND;
    }

    rawrcb_init(&rrawrcb);
    rrawrcb.cb_id = rawrcb->cb_id;
    pd::pd_rawrcb_get_args_init(&pd_rawrcb_args);
    pd_rawrcb_args.rawrcb = &rrawrcb;

    pd_func_args.pd_rawrcb_get = &pd_rawrcb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_RAWRCB_GET, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD RAWRCB: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill config spec of this RAWR CB
    rsp->mutable_spec()->mutable_key_or_handle()->set_rawrcb_id(rrawrcb.cb_id);

    rsp->mutable_spec()->set_rawrcb_flags(rrawrcb.rawrcb_flags);
    rsp->mutable_spec()->set_chain_rxq_base(rrawrcb.chain_rxq_base);
    rsp->mutable_spec()->set_chain_rxq_ring_indices_addr(rrawrcb.chain_rxq_ring_indices_addr);
    rsp->mutable_spec()->set_chain_rxq_ring_size_shift(rrawrcb.chain_rxq_ring_size_shift);
    rsp->mutable_spec()->set_chain_rxq_entry_size_shift(rrawrcb.chain_rxq_entry_size_shift);
    rsp->mutable_spec()->set_chain_rxq_ring_index_select(rrawrcb.chain_rxq_ring_index_select);

    rsp->mutable_spec()->set_chain_txq_base(rrawrcb.chain_txq_base);
    rsp->mutable_spec()->set_chain_txq_ring_indices_addr(rrawrcb.chain_txq_ring_indices_addr);
    rsp->mutable_spec()->set_chain_txq_ring_size_shift(rrawrcb.chain_txq_ring_size_shift);
    rsp->mutable_spec()->set_chain_txq_entry_size_shift(rrawrcb.chain_txq_entry_size_shift);
    rsp->mutable_spec()->set_chain_txq_ring_index_select(rrawrcb.chain_txq_ring_index_select);
    rsp->mutable_spec()->set_chain_txq_lif(rrawrcb.chain_txq_lif);
    rsp->mutable_spec()->set_chain_txq_qtype(rrawrcb.chain_txq_qtype);
    rsp->mutable_spec()->set_chain_txq_qid(rrawrcb.chain_txq_qid);
    rsp->mutable_spec()->set_cpu_id(rrawrcb.cpu_id);
    rsp->mutable_spec()->set_ascq_base(rrawrcb.ascq_base);
    rsp->mutable_spec()->set_ascq_sem_inf_addr(rrawrcb.ascq_sem_inf_addr);

    rsp->mutable_spec()->set_redir_pkts(rrawrcb.redir_pkts);
    rsp->mutable_spec()->set_cb_not_ready_discards(rrawrcb.cb_not_ready_discards);
    rsp->mutable_spec()->set_qstate_cfg_discards(rrawrcb.qstate_cfg_discards);
    rsp->mutable_spec()->set_pkt_len_discards(rrawrcb.pkt_len_discards);
    rsp->mutable_spec()->set_rxq_full_discards(rrawrcb.rxq_full_discards);
    rsp->mutable_spec()->set_txq_full_discards(rrawrcb.txq_full_discards);
    rsp->mutable_spec()->set_pkt_alloc_errors(rrawrcb.pkt_alloc_errors);
    rsp->mutable_spec()->set_pkt_free_errors(rrawrcb.pkt_free_errors);

    // fill operational state of this RAWR CB
    rsp->mutable_status()->set_rawrcb_handle(rawrcb->hal_handle);

    // fill stats of this RAWR CB
    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a RAWR CB delete request
//------------------------------------------------------------------------------
hal_ret_t
rawrcb_delete (internal::RawrCbDeleteRequest& req, internal::RawrCbDeleteResponseMsg *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;
    rawrcb_t*               rawrcb;
    pd::pd_rawrcb_args_t    pd_rawrcb_args;
    pd::pd_rawrcb_delete_args_t    del_args;
    pd::pd_func_args_t          pd_func_args = {0};

    auto kh = req.key_or_handle();
    rawrcb = find_rawrcb_by_id(kh.rawrcb_id());
    if (rawrcb == NULL) {
        rsp->add_api_status(types::API_STATUS_OK);
        return HAL_RET_OK;
    }

    pd::pd_rawrcb_delete_args_init(&del_args);
    del_args.r_args = &pd_rawrcb_args;

    pd_rawrcb_args.rawrcb = rawrcb;

    pd_func_args.pd_rawrcb_delete = &del_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_RAWRCB_DELETE, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD RAWRCB: delete Failed, err: {}", ret);
        rsp->add_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill stats of this RAWR CB
    rsp->add_api_status(types::API_STATUS_OK);

    return HAL_RET_OK;
}

}    // namespace hal
