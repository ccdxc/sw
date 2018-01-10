#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/rawccb.hpp"
#include "nic/hal/src/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/app_redir_shared.h"

namespace hal {
void *
rawccb_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((rawccb_t *)entry)->cb_id);
}

uint32_t
rawccb_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(rawccb_id_t)) % ht_size;
}

bool
rawccb_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(rawccb_id_t *)key1 == *(rawccb_id_t *)key2) {
        return true;
    }
    return false;
}

void *
rawccb_get_handle_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((rawccb_t *)entry)->hal_handle);
}

uint32_t
rawccb_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
rawccb_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
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
    g_hal_state->rawccb_hal_handle_ht()->insert(rawccb,
                                                &rawccb->hal_handle_ht_ctxt);
    g_hal_state->rawccb_id_ht()->insert(rawccb, &rawccb->ht_ctxt);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a RAWC CB create request
// TODO: if RAWC CB exists, treat this as modify (vrf id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
rawccb_create (RawcCbSpec& spec, RawcCbResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;
    rawccb_t                *rawccb = NULL;
    pd::pd_rawccb_args_t    pd_rawccb_args;

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
    rawccb->chain_txq_ring_size_shift = spec.chain_txq_ring_size_shift();
    rawccb->chain_txq_entry_size_shift = spec.chain_txq_entry_size_shift();
    rawccb->chain_txq_lif = spec.chain_txq_lif();
    rawccb->chain_txq_qtype = spec.chain_txq_qtype();
    rawccb->chain_txq_qid = spec.chain_txq_qid();
    rawccb->chain_txq_ring = spec.chain_txq_ring();

    rawccb->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming
    pd::pd_rawccb_args_init(&pd_rawccb_args);
    pd_rawccb_args.rawccb = rawccb;
    ret = pd::pd_rawccb_create(&pd_rawccb_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD RAWC CB create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // add this RAWC CB to our db
    ret = add_rawccb_to_db(rawccb);
    HAL_ASSERT(ret == HAL_RET_OK);

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
rawccb_update (RawcCbSpec& spec, RawcCbResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK; 
    rawccb_t*               rawccb;
    pd::pd_rawccb_args_t    pd_rawccb_args;

    auto kh = spec.key_or_handle();

    rawccb = find_rawccb_by_id(kh.rawccb_id());
    if (rawccb == NULL) {
        rsp->set_api_status(types::API_STATUS_RAWC_CB_NOT_FOUND);
        return HAL_RET_RAWC_CB_NOT_FOUND;
    }
    
    rawccb->my_txq_base = spec.my_txq_base(); 
    rawccb->my_txq_ring_size_shift = spec.my_txq_ring_size_shift();
    rawccb->my_txq_entry_size_shift = spec.my_txq_entry_size_shift();

    rawccb->rawccb_flags = spec.rawccb_flags(); 
    rawccb->chain_txq_base = spec.chain_txq_base(); 
    rawccb->chain_txq_ring_indices_addr = spec.chain_txq_ring_indices_addr(); 
    rawccb->chain_txq_ring_size_shift = spec.chain_txq_ring_size_shift();
    rawccb->chain_txq_entry_size_shift = spec.chain_txq_entry_size_shift();
    rawccb->chain_txq_lif = spec.chain_txq_lif();
    rawccb->chain_txq_qtype = spec.chain_txq_qtype();
    rawccb->chain_txq_qid = spec.chain_txq_qid();
    rawccb->chain_txq_ring = spec.chain_txq_ring();

    pd::pd_rawccb_args_init(&pd_rawccb_args);
    pd_rawccb_args.rawccb = rawccb;
    
    ret = pd::pd_rawccb_update(&pd_rawccb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD RAWCCB: Update Failed, err: ", ret);
        rsp->set_api_status(types::API_STATUS_RAWC_CB_NOT_FOUND);
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
rawccb_get (RawcCbGetRequest& req, RawcCbGetResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK; 
    rawccb_t                rrawccb;
    rawccb_t*               rawccb;
    pd::pd_rawccb_args_t    pd_rawccb_args;

    auto kh = req.key_or_handle();

    rawccb = find_rawccb_by_id(kh.rawccb_id());
    if (rawccb == NULL) {
        rsp->set_api_status(types::API_STATUS_RAWC_CB_NOT_FOUND);
        return HAL_RET_RAWC_CB_NOT_FOUND;
    }
    
    rawccb_init(&rrawccb);
    rrawccb.cb_id = rawccb->cb_id;
    pd::pd_rawccb_args_init(&pd_rawccb_args);
    pd_rawccb_args.rawccb = &rrawccb;
    
    ret = pd::pd_rawccb_get(&pd_rawccb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD RAWCCB: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_RAWC_CB_NOT_FOUND);
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
    rsp->mutable_spec()->set_chain_txq_ring_size_shift(rrawccb.chain_txq_ring_size_shift);
    rsp->mutable_spec()->set_chain_txq_entry_size_shift(rrawccb.chain_txq_entry_size_shift);
    rsp->mutable_spec()->set_chain_txq_lif(rrawccb.chain_txq_lif);
    rsp->mutable_spec()->set_chain_txq_qtype(rrawccb.chain_txq_qtype);
    rsp->mutable_spec()->set_chain_txq_qid(rrawccb.chain_txq_qid);
    rsp->mutable_spec()->set_chain_txq_ring(rrawccb.chain_txq_ring);

    rsp->mutable_spec()->set_stat_pkts_chain(rrawccb.stat_pkts_chain);
    rsp->mutable_spec()->set_stat_pkts_discard(rrawccb.stat_pkts_discard);
    rsp->mutable_spec()->set_stat_cb_not_ready(rrawccb.stat_cb_not_ready);
    rsp->mutable_spec()->set_stat_my_txq_empty(rrawccb.stat_my_txq_empty);
    rsp->mutable_spec()->set_stat_aol_err(rrawccb.stat_aol_err);
    rsp->mutable_spec()->set_stat_txq_full(rrawccb.stat_txq_full);
    rsp->mutable_spec()->set_stat_desc_sem_free_full(rrawccb.stat_desc_sem_free_full);
    rsp->mutable_spec()->set_stat_page_sem_free_full(rrawccb.stat_page_sem_free_full);

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
rawccb_delete (rawccb::RawcCbDeleteRequest& req, rawccb::RawcCbDeleteResponseMsg *rsp)
{
    hal_ret_t               ret = HAL_RET_OK; 
    rawccb_t*               rawccb;
    pd::pd_rawccb_args_t    pd_rawccb_args;

    auto kh = req.key_or_handle();
    rawccb = find_rawccb_by_id(kh.rawccb_id());
    if (rawccb == NULL) {
        rsp->add_api_status(types::API_STATUS_OK);
        return HAL_RET_OK;
    }
 
    pd::pd_rawccb_args_init(&pd_rawccb_args);
    pd_rawccb_args.rawccb = rawccb;
    
    ret = pd::pd_rawccb_delete(&pd_rawccb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD RAWCCB: delete Failed, err: {}", ret);
        rsp->add_api_status(types::API_STATUS_RAWC_CB_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }
    
    // fill stats of this RAWC CB
    rsp->add_api_status(types::API_STATUS_OK);
 
    return HAL_RET_OK;
}

}    // namespace hal
