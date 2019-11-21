//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/src/internal/rawccb.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/app_redir_shared.h"

namespace hal {
void *
rawccb_get_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((rawccb_t *)entry)->cb_id);
}

uint32_t
rawccb_key_size ()
{
    return sizeof(rawccb_id_t);
}

void *
rawccb_get_handle_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((rawccb_t *)entry)->hal_handle);
}

uint32_t
rawccb_handle_key_size ()
{
    return sizeof(hal_handle_t);
}

// allocate a RAWCCB instance
static inline rawccb_t *
rawccb_alloc (void)
{
    rawccb_t    *rawccb;

    rawccb = (rawccb_t *)g_hal_state->rawccb_slab()->alloc();
    if (rawccb == NULL) {
        return NULL;
    }
    return rawccb;
}

// initialize a RAWCCB instance
static inline rawccb_t *
rawccb_init (rawccb_t *rawccb)
{
    if (!rawccb) {
        return NULL;
    }
    SDK_SPINLOCK_INIT(&rawccb->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    rawccb->pd = NULL;

    // initialize meta information
    rawccb->ht_ctxt.reset();
    rawccb->hal_handle_ht_ctxt.reset();

    return rawccb;
}

// allocate and initialize a RAWCCB instance
static inline rawccb_t *
rawccb_alloc_init (void)
{
    return rawccb_init(rawccb_alloc());
}

static inline hal_ret_t
rawccb_free (rawccb_t *rawccb)
{
    SDK_SPINLOCK_DESTROY(&rawccb->slock);
    hal::delay_delete_to_slab(HAL_SLAB_RAWCCB, rawccb);
    return HAL_RET_OK;
}

static inline rawccb_t *
find_rawccb_by_id (rawccb_id_t rawccb_id)
{
    return (rawccb_t *)g_hal_state->rawccb_id_ht()->lookup(&rawccb_id);
}

//------------------------------------------------------------------------------
// validate an incoming RAWCCB create request
// TODO:
// 1. check if RAWCCB exists already
//------------------------------------------------------------------------------
static hal_ret_t
validate_rawccb_create (RawcCbSpec& spec, RawcCbResponse *rsp)
{
    // must have key-handle set
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_RAWC_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    // must have key in the key-handle
    if (spec.key_or_handle().key_or_handle_case() !=
            RawcCbKeyHandle::kRawccbId) {
        rsp->set_api_status(types::API_STATUS_RAWC_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    if (spec.key_or_handle().rawccb_id() >= RAWCCB_NUM_ENTRIES_MAX) {
        rsp->set_api_status(types::API_STATUS_RAWC_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert this RAWC CB in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_rawccb_to_db (rawccb_t *rawccb)
{
    g_hal_state->rawccb_id_ht()->insert(rawccb, &rawccb->ht_ctxt);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a RAWC CB create request
// TODO: if RAWC CB exists, treat this as modify (vrf id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
rawccb_create (internal::RawcCbSpec& spec, internal::RawcCbResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;
    rawccb_t                *rawccb = NULL;
    pd::pd_rawccb_create_args_t    pd_rawccb_args;
    pd::pd_func_args_t          pd_func_args = {0};

    // validate the request message
    ret = validate_rawccb_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        goto cleanup;
    }

    // instantiate RAWC CB
    rawccb = rawccb_alloc_init();
    if (rawccb == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
        goto cleanup;
    }

    rawccb->cb_id = spec.key_or_handle().rawccb_id();
    rawccb->rawccb_flags = spec.rawccb_flags();
    rawccb->my_txq_base = spec.my_txq_base();
    rawccb->my_txq_ring_size_shift = spec.my_txq_ring_size_shift();
    rawccb->my_txq_entry_size_shift = spec.my_txq_entry_size_shift();

    rawccb->chain_txq_base = spec.chain_txq_base();
    rawccb->chain_txq_ring_indices_addr = spec.chain_txq_ring_indices_addr();
    rawccb->ascq_base = spec.ascq_base();
    rawccb->ascq_sem_inf_addr = spec.ascq_sem_inf_addr();
    rawccb->chain_txq_ring_size_shift = spec.chain_txq_ring_size_shift();
    rawccb->chain_txq_entry_size_shift = spec.chain_txq_entry_size_shift();
    rawccb->chain_txq_lif = spec.chain_txq_lif();
    rawccb->chain_txq_qtype = spec.chain_txq_qtype();
    rawccb->chain_txq_qid = spec.chain_txq_qid();
    rawccb->chain_txq_ring = spec.chain_txq_ring();
    rawccb->cpu_id = spec.cpu_id();

    rawccb->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming
    pd::pd_rawccb_create_args_init(&pd_rawccb_args);
    pd_rawccb_args.rawccb = rawccb;
    pd_func_args.pd_rawccb_create = &pd_rawccb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_RAWCCB_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD RAWC CB create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // add this RAWC CB to our db
    ret = add_rawccb_to_db(rawccb);
    SDK_ASSERT(ret == HAL_RET_OK);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_rawccb_status()->set_rawccb_handle(rawccb->hal_handle);
    return HAL_RET_OK;

cleanup:

    if (rawccb) {
        rawccb_free(rawccb);
    }
    return ret;
}

//------------------------------------------------------------------------------
// process a RAWC CB update request
//------------------------------------------------------------------------------
hal_ret_t
rawccb_update (internal::RawcCbSpec& spec, internal::RawcCbResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;
    rawccb_t*               rawccb;
    pd::pd_rawccb_update_args_t    pd_rawccb_args;
    pd::pd_func_args_t          pd_func_args = {0};

    auto kh = spec.key_or_handle();

    rawccb = find_rawccb_by_id(kh.rawccb_id());
    if (rawccb == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_RAWC_CB_NOT_FOUND;
    }

    rawccb->my_txq_base = spec.my_txq_base();
    rawccb->my_txq_ring_size_shift = spec.my_txq_ring_size_shift();
    rawccb->my_txq_entry_size_shift = spec.my_txq_entry_size_shift();

    rawccb->rawccb_flags = spec.rawccb_flags();
    rawccb->chain_txq_base = spec.chain_txq_base();
    rawccb->chain_txq_ring_indices_addr = spec.chain_txq_ring_indices_addr();
    rawccb->ascq_base = spec.ascq_base();
    rawccb->ascq_sem_inf_addr = spec.ascq_sem_inf_addr();
    rawccb->chain_txq_ring_size_shift = spec.chain_txq_ring_size_shift();
    rawccb->chain_txq_entry_size_shift = spec.chain_txq_entry_size_shift();
    rawccb->chain_txq_lif = spec.chain_txq_lif();
    rawccb->chain_txq_qtype = spec.chain_txq_qtype();
    rawccb->chain_txq_qid = spec.chain_txq_qid();
    rawccb->chain_txq_ring = spec.chain_txq_ring();
    rawccb->cpu_id = spec.cpu_id();

    pd::pd_rawccb_update_args_init(&pd_rawccb_args);
    pd_rawccb_args.rawccb = rawccb;

    pd_func_args.pd_rawccb_update = &pd_rawccb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_RAWCCB_UPDATE, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD RAWCCB: Update Failed, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill stats of this RAWC CB
    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a RAWC CB get request
//------------------------------------------------------------------------------
hal_ret_t
rawccb_get (internal::RawcCbGetRequest& req, internal::RawcCbGetResponseMsg *resp)
{
    hal_ret_t               ret = HAL_RET_OK;
    rawccb_t                rrawccb;
    rawccb_t*               rawccb;
    pd::pd_rawccb_get_args_t    pd_rawccb_args;
    pd::pd_func_args_t          pd_func_args = {0};
    RawcCbGetResponse *rsp = resp->add_response();

    auto kh = req.key_or_handle();

    rawccb = find_rawccb_by_id(kh.rawccb_id());
    if (rawccb == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_RAWC_CB_NOT_FOUND;
    }

    rawccb_init(&rrawccb);
    rrawccb.cb_id = rawccb->cb_id;
    pd::pd_rawccb_get_args_init(&pd_rawccb_args);
    pd_rawccb_args.rawccb = &rrawccb;

    pd_func_args.pd_rawccb_get = &pd_rawccb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_RAWCCB_GET, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD RAWCCB: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill config spec of this RAWC CB
    rsp->mutable_spec()->mutable_key_or_handle()->set_rawccb_id(rrawccb.cb_id);

    rsp->mutable_spec()->set_rawccb_flags(rrawccb.rawccb_flags);
    rsp->mutable_spec()->set_my_txq_base(rrawccb.my_txq_base);
    rsp->mutable_spec()->set_my_txq_ring_size_shift(rrawccb.my_txq_ring_size_shift);
    rsp->mutable_spec()->set_my_txq_entry_size_shift(rrawccb.my_txq_entry_size_shift);

    rsp->mutable_spec()->set_chain_txq_base(rrawccb.chain_txq_base);
    rsp->mutable_spec()->set_chain_txq_ring_indices_addr(rrawccb.chain_txq_ring_indices_addr);
    rsp->mutable_spec()->set_ascq_base(rrawccb.ascq_base);
    rsp->mutable_spec()->set_ascq_sem_inf_addr(rrawccb.ascq_sem_inf_addr);
    rsp->mutable_spec()->set_chain_txq_ring_size_shift(rrawccb.chain_txq_ring_size_shift);
    rsp->mutable_spec()->set_chain_txq_entry_size_shift(rrawccb.chain_txq_entry_size_shift);
    rsp->mutable_spec()->set_chain_txq_lif(rrawccb.chain_txq_lif);
    rsp->mutable_spec()->set_chain_txq_qtype(rrawccb.chain_txq_qtype);
    rsp->mutable_spec()->set_chain_txq_qid(rrawccb.chain_txq_qid);
    rsp->mutable_spec()->set_chain_txq_ring(rrawccb.chain_txq_ring);
    rsp->mutable_spec()->set_cpu_id(rrawccb.cpu_id);

    rsp->mutable_spec()->set_chain_pkts(rrawccb.chain_pkts);
    rsp->mutable_spec()->set_cb_not_ready_discards(rrawccb.cb_not_ready_discards);
    rsp->mutable_spec()->set_qstate_cfg_discards(rrawccb.qstate_cfg_discards);
    rsp->mutable_spec()->set_aol_error_discards(rrawccb.aol_error_discards);
    rsp->mutable_spec()->set_my_txq_empty_discards(rrawccb.my_txq_empty_discards);
    rsp->mutable_spec()->set_txq_full_discards(rrawccb.txq_full_discards);
    rsp->mutable_spec()->set_pkt_free_errors(rrawccb.pkt_free_errors);

    rsp->mutable_spec()->set_pi(rrawccb.pi);
    rsp->mutable_spec()->set_ci(rrawccb.ci);

    // fill operational state of this RAWC CB
    rsp->mutable_status()->set_rawccb_handle(rawccb->hal_handle);

    // fill stats of this RAWC CB
    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a RAWC CB delete request
//------------------------------------------------------------------------------
hal_ret_t
rawccb_delete (internal::RawcCbDeleteRequest& req, internal::RawcCbDeleteResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;
    rawccb_t*               rawccb;
    pd::pd_rawccb_args_t    pd_rawccb_args;
    pd::pd_rawccb_delete_args_t    del_args;
    pd::pd_func_args_t          pd_func_args = {0};

    auto kh = req.key_or_handle();
    rawccb = find_rawccb_by_id(kh.rawccb_id());
    if (rawccb == NULL) {
        rsp->set_api_status(types::API_STATUS_OK);
        return HAL_RET_OK;
    }

    pd::pd_rawccb_delete_args_init(&del_args);
    del_args.r_args = &pd_rawccb_args;
    pd_rawccb_args.rawccb = rawccb;

    pd_func_args.pd_rawccb_delete = &del_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_RAWCCB_DELETE, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD RAWCCB: delete Failed, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill stats of this RAWC CB
    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

}   // namespace hal
