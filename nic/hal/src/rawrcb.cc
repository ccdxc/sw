#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/rawrcb.hpp"
// #include "nic/hal/svc/rawrcb_svc.hpp"
#include "nic/hal/src/tenant.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {
void *
rawrcb_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((rawrcb_t *)entry)->cb_id);
}

uint32_t
rawrcb_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(rawrcb_id_t)) % ht_size;
}

bool
rawrcb_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(rawrcb_id_t *)key1 == *(rawrcb_id_t *)key2) {
        return true;
    }
    return false;
}

void *
rawrcb_get_handle_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((rawrcb_t *)entry)->hal_handle);
}

uint32_t
rawrcb_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
rawrcb_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
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
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert this RAWR CB in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_rawrcb_to_db (rawrcb_t *rawrcb)
{
    g_hal_state->rawrcb_hal_handle_ht()->insert(rawrcb,
                                               &rawrcb->hal_handle_ht_ctxt);
    g_hal_state->rawrcb_id_ht()->insert(rawrcb, &rawrcb->ht_ctxt);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a RAWR CB create request
// TODO: if RAWR CB exists, treat this as modify (tenant id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
rawrcb_create (RawrCbSpec& spec, RawrCbResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;
    rawrcb_t                *rawrcb;
    pd::pd_rawrcb_args_t    pd_rawrcb_args;

    // validate the request message
    ret = validate_rawrcb_create(spec, rsp);
    
    // instantiate RAWR CB
    rawrcb = rawrcb_alloc_init();
    if (rawrcb == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }

    rawrcb->cb_id = spec.key_or_handle().rawrcb_id();
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
    rawrcb->chain_txq_doorbell_no_sched = spec.chain_txq_doorbell_no_sched();

    rawrcb->desc_valid_bit_upd = spec.desc_valid_bit_upd();
    rawrcb->desc_valid_bit_req = spec.desc_valid_bit_req();
    rawrcb->redir_pipeline_lpbk_enable = spec.redir_pipeline_lpbk_enable();

    rawrcb->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming
    pd::pd_rawrcb_args_init(&pd_rawrcb_args);
    pd_rawrcb_args.rawrcb = rawrcb;
    ret = pd::pd_rawrcb_create(&pd_rawrcb_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD RAWR CB create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // add this RAWR CB to our db
    ret = add_rawrcb_to_db(rawrcb);
    HAL_ASSERT(ret == HAL_RET_OK);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_rawrcb_status()->set_rawrcb_handle(rawrcb->hal_handle);
    return HAL_RET_OK;

cleanup:

    rawrcb_free(rawrcb);
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
    pd::pd_rawrcb_args_t    pd_rawrcb_args;

    auto kh = spec.key_or_handle();

    rawrcb = find_rawrcb_by_id(kh.rawrcb_id());
    if (rawrcb == NULL) {
        rsp->set_api_status(types::API_STATUS_RAWR_CB_NOT_FOUND);
        return HAL_RET_RAWR_CB_NOT_FOUND;
    }
    
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
    rawrcb->chain_txq_doorbell_no_sched = spec.chain_txq_doorbell_no_sched();

    rawrcb->desc_valid_bit_upd = spec.desc_valid_bit_upd();
    rawrcb->desc_valid_bit_req = spec.desc_valid_bit_req();
    rawrcb->redir_pipeline_lpbk_enable = spec.redir_pipeline_lpbk_enable();

    pd::pd_rawrcb_args_init(&pd_rawrcb_args);
    pd_rawrcb_args.rawrcb = rawrcb;
    
    ret = pd::pd_rawrcb_update(&pd_rawrcb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD RAWRCB: Update Failed, err: ", ret);
        rsp->set_api_status(types::API_STATUS_RAWR_CB_NOT_FOUND);
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
rawrcb_get (RawrCbGetRequest& req, RawrCbGetResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK; 
    rawrcb_t                rrawrcb;
    rawrcb_t*               rawrcb;
    pd::pd_rawrcb_args_t    pd_rawrcb_args;

    auto kh = req.key_or_handle();

    rawrcb = find_rawrcb_by_id(kh.rawrcb_id());
    if (rawrcb == NULL) {
        rsp->set_api_status(types::API_STATUS_RAWR_CB_NOT_FOUND);
        return HAL_RET_RAWR_CB_NOT_FOUND;
    }
    
    rawrcb_init(&rrawrcb);
    rrawrcb.cb_id = rawrcb->cb_id;
    pd::pd_rawrcb_args_init(&pd_rawrcb_args);
    pd_rawrcb_args.rawrcb = &rrawrcb;
    
    ret = pd::pd_rawrcb_get(&pd_rawrcb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD RAWRCB: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_RAWR_CB_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill config spec of this RAWR CB 
    rsp->mutable_spec()->mutable_key_or_handle()->set_rawrcb_id(rrawrcb.cb_id);
    
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
    rsp->mutable_spec()->set_chain_txq_doorbell_no_sched(rrawrcb.chain_txq_doorbell_no_sched);

    rsp->mutable_spec()->set_desc_valid_bit_upd(rrawrcb.desc_valid_bit_upd);
    rsp->mutable_spec()->set_desc_valid_bit_req(rrawrcb.desc_valid_bit_req);
    rsp->mutable_spec()->set_redir_pipeline_lpbk_enable(rrawrcb.redir_pipeline_lpbk_enable);

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
rawrcb_delete (rawrcb::RawrCbDeleteRequest& req, rawrcb::RawrCbDeleteResponseMsg *rsp)
{
    hal_ret_t               ret = HAL_RET_OK; 
    rawrcb_t*               rawrcb;
    pd::pd_rawrcb_args_t    pd_rawrcb_args;

    auto kh = req.key_or_handle();
    rawrcb = find_rawrcb_by_id(kh.rawrcb_id());
    if (rawrcb == NULL) {
        rsp->add_api_status(types::API_STATUS_OK);
        return HAL_RET_OK;
    }
 
    pd::pd_rawrcb_args_init(&pd_rawrcb_args);
    pd_rawrcb_args.rawrcb = rawrcb;
    
    ret = pd::pd_rawrcb_delete(&pd_rawrcb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD RAWRCB: delete Failed, err: {}", ret);
        rsp->add_api_status(types::API_STATUS_RAWR_CB_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }
    
    // fill stats of this RAWR CB
    rsp->add_api_status(types::API_STATUS_OK);
 
    return HAL_RET_OK;
}

}    // namespace hal
