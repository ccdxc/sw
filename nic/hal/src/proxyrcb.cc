#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/proxyrcb.hpp"
#include "nic/hal/src/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/app_redir_shared.h"

namespace hal {
void *
proxyrcb_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((proxyrcb_t *)entry)->cb_id);
}

uint32_t
proxyrcb_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(proxyrcb_id_t)) % ht_size;
}

bool
proxyrcb_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(proxyrcb_id_t *)key1 == *(proxyrcb_id_t *)key2) {
        return true;
    }
    return false;
}

void *
proxyrcb_get_handle_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((proxyrcb_t *)entry)->hal_handle);
}

uint32_t
proxyrcb_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
proxyrcb_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// validate an incoming PROXYRCB create request
// TODO:
// 1. check if PROXYRCB exists already
//------------------------------------------------------------------------------
static hal_ret_t
validate_proxyrcb_create (ProxyrCbSpec& spec, ProxyrCbResponse *rsp)
{
    // must have key-handle set
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_PROXYR_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    // must have key in the key-handle
    if (spec.key_or_handle().key_or_handle_case() !=
            ProxyrCbKeyHandle::kProxyrcbId) {
        rsp->set_api_status(types::API_STATUS_PROXYR_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    if (spec.key_or_handle().proxyrcb_id() >= 
             (PROXYRCB_NUM_ENTRIES_MAX * PROXYRCB_NUM_ENTRIES_MAX_MULT)) {
        rsp->set_api_status(types::API_STATUS_PROXYR_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert this PROXYR CB in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_proxyrcb_to_db (proxyrcb_t *proxyrcb)
{
    g_hal_state->proxyrcb_hal_handle_ht()->insert(proxyrcb,
                                               &proxyrcb->hal_handle_ht_ctxt);
    g_hal_state->proxyrcb_id_ht()->insert(proxyrcb, &proxyrcb->ht_ctxt);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a PROXYR CB create request
// TODO: if PROXYR CB exists, treat this as modify (vrf id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
proxyrcb_create (ProxyrCbSpec& spec, ProxyrCbResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;
    proxyrcb_t              *proxyrcb = NULL;
    pd::pd_proxyrcb_args_t  pd_proxyrcb_args;

    // validate the request message
    ret = validate_proxyrcb_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        goto cleanup;
    }
    
    // instantiate PROXYR CB
    proxyrcb = proxyrcb_alloc_init();
    if (proxyrcb == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
        goto cleanup;
    }

    proxyrcb->cb_id = spec.key_or_handle().proxyrcb_id();
    proxyrcb->proxyrcb_flags = spec.proxyrcb_flags(); 
    proxyrcb->my_txq_base = spec.my_txq_base(); 
    proxyrcb->my_txq_ring_size_shift = spec.my_txq_ring_size_shift();
    proxyrcb->my_txq_entry_size_shift = spec.my_txq_entry_size_shift();
    proxyrcb->chain_rxq_base = spec.chain_rxq_base(); 
    proxyrcb->chain_rxq_ring_indices_addr = spec.chain_rxq_ring_indices_addr(); 
    proxyrcb->chain_rxq_ring_size_shift = spec.chain_rxq_ring_size_shift();
    proxyrcb->chain_rxq_entry_size_shift = spec.chain_rxq_entry_size_shift();
    proxyrcb->chain_rxq_ring_index_select = spec.chain_rxq_ring_index_select();

    if (spec.af() == AF_INET) {
        proxyrcb->ip_sa.v4_addr = spec.ip_sa().v4_addr();
        proxyrcb->ip_da.v4_addr = spec.ip_da().v4_addr();
    } else {
        memcpy(proxyrcb->ip_sa.v6_addr.addr8,
               spec.ip_sa().v6_addr().c_str(),
               IP6_ADDR8_LEN);
        memcpy(proxyrcb->ip_da.v6_addr.addr8,
               spec.ip_da().v6_addr().c_str(),
               IP6_ADDR8_LEN);
    }
    proxyrcb->sport = spec.sport();
    proxyrcb->dport = spec.dport();
    proxyrcb->vrf = spec.vrf();
    proxyrcb->af = spec.af();
    proxyrcb->ip_proto = spec.ip_proto();

    proxyrcb->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming
    pd::pd_proxyrcb_args_init(&pd_proxyrcb_args);
    pd_proxyrcb_args.proxyrcb = proxyrcb;
    ret = pd::pd_proxyrcb_create(&pd_proxyrcb_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD PROXYR CB create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // add this PROXYR CB to our db
    ret = add_proxyrcb_to_db(proxyrcb);
    HAL_ASSERT(ret == HAL_RET_OK);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_proxyrcb_status()->set_proxyrcb_handle(proxyrcb->hal_handle);
    return HAL_RET_OK;

cleanup:

    if (proxyrcb) {
        proxyrcb_free(proxyrcb);
    }
    return ret;
}

//------------------------------------------------------------------------------
// process a PROXYR CB update request
//------------------------------------------------------------------------------
hal_ret_t
proxyrcb_update (ProxyrCbSpec& spec, ProxyrCbResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK; 
    proxyrcb_t              *proxyrcb;
    pd::pd_proxyrcb_args_t  pd_proxyrcb_args;

    auto kh = spec.key_or_handle();

    proxyrcb = find_proxyrcb_by_id(kh.proxyrcb_id());
    if (proxyrcb == NULL) {
        rsp->set_api_status(types::API_STATUS_PROXYR_CB_NOT_FOUND);
        return HAL_RET_PROXYR_CB_NOT_FOUND;
    }
    
    proxyrcb->proxyrcb_flags = spec.proxyrcb_flags(); 
    proxyrcb->my_txq_base = spec.my_txq_base(); 
    proxyrcb->my_txq_ring_size_shift = spec.my_txq_ring_size_shift();
    proxyrcb->my_txq_entry_size_shift = spec.my_txq_entry_size_shift();
    proxyrcb->chain_rxq_base = spec.chain_rxq_base(); 
    proxyrcb->chain_rxq_ring_indices_addr = spec.chain_rxq_ring_indices_addr(); 
    proxyrcb->chain_rxq_ring_size_shift = spec.chain_rxq_ring_size_shift();
    proxyrcb->chain_rxq_entry_size_shift = spec.chain_rxq_entry_size_shift();
    proxyrcb->chain_rxq_ring_index_select = spec.chain_rxq_ring_index_select();

    if (spec.af() == AF_INET) {
        proxyrcb->ip_sa.v4_addr = spec.ip_sa().v4_addr();
        proxyrcb->ip_da.v4_addr = spec.ip_da().v4_addr();
    } else {
        memcpy(proxyrcb->ip_sa.v6_addr.addr8,
               spec.ip_sa().v6_addr().c_str(),
               IP6_ADDR8_LEN);
        memcpy(proxyrcb->ip_da.v6_addr.addr8,
               spec.ip_da().v6_addr().c_str(),
               IP6_ADDR8_LEN);
    }
    proxyrcb->sport = spec.sport();
    proxyrcb->dport = spec.dport();
    proxyrcb->vrf = spec.vrf();
    proxyrcb->af = spec.af();
    proxyrcb->ip_proto = spec.ip_proto();

    pd::pd_proxyrcb_args_init(&pd_proxyrcb_args);
    pd_proxyrcb_args.proxyrcb = proxyrcb;
    
    ret = pd::pd_proxyrcb_update(&pd_proxyrcb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD PROXYRCB: Update Failed, err: ", ret);
        rsp->set_api_status(types::API_STATUS_PROXYR_CB_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }
    
    // fill stats of this PROXYR CB
    rsp->set_api_status(types::API_STATUS_OK);
 
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a PROXYR CB get request
//------------------------------------------------------------------------------
hal_ret_t
proxyrcb_get (ProxyrCbGetRequest& req, ProxyrCbGetResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK; 
    proxyrcb_t              rproxyrcb;
    proxyrcb_t              *proxyrcb;
    pd::pd_proxyrcb_args_t  pd_proxyrcb_args;

    auto kh = req.key_or_handle();

    proxyrcb = find_proxyrcb_by_id(kh.proxyrcb_id());
    if (proxyrcb == NULL) {
        rsp->set_api_status(types::API_STATUS_PROXYR_CB_NOT_FOUND);
        return HAL_RET_PROXYR_CB_NOT_FOUND;
    }
    
    proxyrcb_init(&rproxyrcb);
    rproxyrcb.cb_id = proxyrcb->cb_id;
    pd::pd_proxyrcb_args_init(&pd_proxyrcb_args);
    pd_proxyrcb_args.proxyrcb = &rproxyrcb;
    
    ret = pd::pd_proxyrcb_get(&pd_proxyrcb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD PROXYRCB: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_PROXYR_CB_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill config spec of this PROXYR CB 
    rsp->mutable_spec()->mutable_key_or_handle()->set_proxyrcb_id(rproxyrcb.cb_id);
    
    rsp->mutable_spec()->set_proxyrcb_flags(rproxyrcb.proxyrcb_flags);
    rsp->mutable_spec()->set_my_txq_base(rproxyrcb.my_txq_base);
    rsp->mutable_spec()->set_my_txq_ring_size_shift(rproxyrcb.my_txq_ring_size_shift);
    rsp->mutable_spec()->set_my_txq_entry_size_shift(rproxyrcb.my_txq_entry_size_shift);
    rsp->mutable_spec()->set_chain_rxq_base(rproxyrcb.chain_rxq_base);
    rsp->mutable_spec()->set_chain_rxq_ring_indices_addr(rproxyrcb.chain_rxq_ring_indices_addr);
    rsp->mutable_spec()->set_chain_rxq_ring_size_shift(rproxyrcb.chain_rxq_ring_size_shift);
    rsp->mutable_spec()->set_chain_rxq_entry_size_shift(rproxyrcb.chain_rxq_entry_size_shift);
    rsp->mutable_spec()->set_chain_rxq_ring_index_select(rproxyrcb.chain_rxq_ring_index_select);

    if (rproxyrcb.af == AF_INET) {
        rsp->mutable_spec()->mutable_ip_sa()->set_v4_addr(rproxyrcb.ip_sa.v4_addr);
        rsp->mutable_spec()->mutable_ip_da()->set_v4_addr(rproxyrcb.ip_da.v4_addr);
    } else {
        rsp->mutable_spec()->mutable_ip_sa()->set_v6_addr(&rproxyrcb.ip_sa.v6_addr,
                                                          IP6_ADDR8_LEN);
        rsp->mutable_spec()->mutable_ip_da()->set_v6_addr(&rproxyrcb.ip_da.v6_addr,
                                                          IP6_ADDR8_LEN);
    }

    rsp->mutable_spec()->set_sport(rproxyrcb.sport);
    rsp->mutable_spec()->set_dport(rproxyrcb.dport);
    rsp->mutable_spec()->set_vrf(rproxyrcb.vrf);
    rsp->mutable_spec()->set_af(rproxyrcb.af);
    rsp->mutable_spec()->set_ip_proto(types::IPProtocol(rproxyrcb.ip_proto));

    rsp->mutable_spec()->set_pi(rproxyrcb.pi);
    rsp->mutable_spec()->set_ci(rproxyrcb.ci);

    // fill operational state of this PROXYR CB
    rsp->mutable_status()->set_proxyrcb_handle(proxyrcb->hal_handle);

    // fill stats of this PROXYR CB
    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a PROXYR CB delete request
//------------------------------------------------------------------------------
hal_ret_t
proxyrcb_delete (proxyrcb::ProxyrCbDeleteRequest& req, proxyrcb::ProxyrCbDeleteResponseMsg *rsp)
{
    hal_ret_t               ret = HAL_RET_OK; 
    proxyrcb_t              *proxyrcb;
    pd::pd_proxyrcb_args_t  pd_proxyrcb_args;

    auto kh = req.key_or_handle();
    proxyrcb = find_proxyrcb_by_id(kh.proxyrcb_id());
    if (proxyrcb == NULL) {
        rsp->add_api_status(types::API_STATUS_OK);
        return HAL_RET_OK;
    }
 
    pd::pd_proxyrcb_args_init(&pd_proxyrcb_args);
    pd_proxyrcb_args.proxyrcb = proxyrcb;
    
    ret = pd::pd_proxyrcb_delete(&pd_proxyrcb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD PROXYRCB: delete Failed, err: {}", ret);
        rsp->add_api_status(types::API_STATUS_PROXYR_CB_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }
    
    // fill stats of this PROXYR CB
    rsp->add_api_status(types::API_STATUS_OK);
 
    return HAL_RET_OK;
}

}    // namespace hal
