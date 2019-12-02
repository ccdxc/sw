//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/src/internal/proxyccb.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/app_redir_shared.h"

namespace hal {
void *
proxyccb_get_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((proxyccb_t *)entry)->cb_id);
}

uint32_t
proxyccb_key_size ()
{
    return sizeof(proxyccb_id_t);
}

void *
proxyccb_get_handle_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((proxyccb_t *)entry)->hal_handle);
}

uint32_t
proxyccb_handle_key_size ()
{
    return sizeof(hal_handle_t);
}

// allocate a PROXYCCB instance
static inline proxyccb_t *
proxyccb_alloc (void)
{
    proxyccb_t    *proxyccb;

    proxyccb = (proxyccb_t *)g_hal_state->proxyccb_slab()->alloc();
    if (proxyccb == NULL) {
        return NULL;
    }
    return proxyccb;
}

// initialize a PROXYCCB instance
static inline proxyccb_t *
proxyccb_init (proxyccb_t *proxyccb)
{
    if (!proxyccb) {
        return NULL;
    }
    SDK_SPINLOCK_INIT(&proxyccb->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    proxyccb->pd = NULL;

    // initialize meta information
    proxyccb->ht_ctxt.reset();
    proxyccb->hal_handle_ht_ctxt.reset();

    return proxyccb;
}

// allocate and initialize a PROXYCCB instance
static inline proxyccb_t *
proxyccb_alloc_init (void)
{
    return proxyccb_init(proxyccb_alloc());
}

static inline hal_ret_t
proxyccb_free (proxyccb_t *proxyccb)
{
    SDK_SPINLOCK_DESTROY(&proxyccb->slock);
    hal::delay_delete_to_slab(HAL_SLAB_PROXYCCB, proxyccb);
    return HAL_RET_OK;
}

static inline proxyccb_t *
find_proxyccb_by_id (proxyccb_id_t proxyccb_id)
{
    return (proxyccb_t *)g_hal_state->proxyccb_id_ht()->lookup(&proxyccb_id);
}

/*
 * A given tcpcb will be enabled for L7 redirect if there's a corresponding
 * proxy chain CB which forwards to SERVICE_LIF_TLS_PROXY. In other words,
 * the packet flow is intended to progress as follows:
 *  TCP proxy -> app_redir (proxyrcb) -> ARM -> app_redir (proxyccb) -> TLS proxy
 */
types::AppRedirType
proxyccb_tcpcb_l7_proxy_type_eval(uint32_t flow_id)
{
    const proxyccb_t *proxyccb = find_proxyccb_by_id(flow_id);

    if (proxyccb && (proxyccb->chain_txq_lif == SERVICE_LIF_TLS_PROXY)) {
        HAL_TRACE_DEBUG("{} enable TCPCB {} for app_redir", __FUNCTION__, flow_id);
        return proxyccb->redir_span ? types::APP_REDIR_TYPE_SPAN :
                                      types::APP_REDIR_TYPE_REDIRECT;
    }

    return types::APP_REDIR_TYPE_NONE;
}

/*
 * A given tlscb will be enabled for L7 redirect if there's a corresponding
 * proxy chain CB which forwards to SERVICE_LIF_TCP_PROXY. In other words,
 * the packet flow is intended to progress as follows:
 *  TLS proxy -> app_redir (proxyrcb) -> ARM -> app_redir (proxyccb) -> TCP proxy
 */
types::AppRedirType
proxyccb_tlscb_l7_proxy_type_eval(uint32_t flow_id)
{
    const proxyccb_t *proxyccb = find_proxyccb_by_id(flow_id);

    if (proxyccb && (proxyccb->chain_txq_lif == SERVICE_LIF_TCP_PROXY)) {
        HAL_TRACE_DEBUG("{} enable TLSCB {} for app_redir", __FUNCTION__, flow_id);
        return proxyccb->redir_span ? types::APP_REDIR_TYPE_SPAN :
                                      types::APP_REDIR_TYPE_REDIRECT;
    }

    return types::APP_REDIR_TYPE_NONE;
}

//------------------------------------------------------------------------------
// validate an incoming PROXYCCB create request
// TODO:
// 1. check if PROXYCCB exists already
//------------------------------------------------------------------------------
static hal_ret_t
validate_proxyccb_create (ProxycCbSpec& spec, ProxycCbResponse *rsp)
{
    // must have key-handle set
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_PROXYC_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    // must have key in the key-handle
    if (spec.key_or_handle().key_or_handle_case() !=
            ProxycCbKeyHandle::kProxyccbId) {
        rsp->set_api_status(types::API_STATUS_PROXYC_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    if (spec.key_or_handle().proxyccb_id() >= PROXYCCB_NUM_ENTRIES_MAX) {
        rsp->set_api_status(types::API_STATUS_PROXYC_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert/delete this PROXYC CB in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_proxyccb_to_db (proxyccb_t *proxyccb)
{
    g_hal_state->proxyccb_id_ht()->insert(proxyccb, &proxyccb->ht_ctxt);
    return HAL_RET_OK;
}

static inline void
del_proxyccb_from_db (proxyccb_t *proxyccb)
{
    g_hal_state->proxyccb_id_ht()->remove_entry(proxyccb, &proxyccb->ht_ctxt);
}

//------------------------------------------------------------------------------
// process a PROXYC CB create request
// TODO: if PROXYC CB exists, treat this as modify (vrf id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
proxyccb_create (internal::ProxycCbSpec& spec, internal::ProxycCbResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;
    proxyccb_t              *proxyccb = NULL;
    pd::pd_proxyccb_create_args_t  pd_proxyccb_args;
    pd::pd_func_args_t      pd_func_args = {0};

    // validate the request message
    ret = validate_proxyccb_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        goto cleanup;
    }

    // instantiate PROXYC CB
    proxyccb = proxyccb_alloc_init();
    if (proxyccb == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
        goto cleanup;
    }

    proxyccb->cb_id = spec.key_or_handle().proxyccb_id();
    proxyccb->proxyccb_flags = spec.proxyccb_flags();
    proxyccb->my_txq_base = spec.my_txq_base();
    proxyccb->my_txq_ring_size_shift = spec.my_txq_ring_size_shift();
    proxyccb->my_txq_entry_size_shift = spec.my_txq_entry_size_shift();

    proxyccb->chain_txq_base = spec.chain_txq_base();
    proxyccb->chain_txq_ring_indices_addr = spec.chain_txq_ring_indices_addr();
    proxyccb->chain_txq_ring_size_shift = spec.chain_txq_ring_size_shift();
    proxyccb->chain_txq_entry_size_shift = spec.chain_txq_entry_size_shift();
    proxyccb->chain_txq_lif = spec.chain_txq_lif();
    proxyccb->chain_txq_qtype = spec.chain_txq_qtype();
    proxyccb->chain_txq_qid = spec.chain_txq_qid();
    proxyccb->chain_txq_ring = spec.chain_txq_ring();

    proxyccb->redir_span = spec.redir_span();
    proxyccb->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming
    pd::pd_proxyccb_create_args_init(&pd_proxyccb_args);
    pd_proxyccb_args.proxyccb = proxyccb;
    pd_func_args.pd_proxyccb_create = &pd_proxyccb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROXYCCB_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD PROXYC CB create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // add this PROXYC CB to our db
    ret = add_proxyccb_to_db(proxyccb);
    SDK_ASSERT(ret == HAL_RET_OK);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_proxyccb_status()->set_proxyccb_handle(proxyccb->hal_handle);
    return HAL_RET_OK;

cleanup:

    if (proxyccb) {
        proxyccb_free(proxyccb);
    }
    return ret;
}

//------------------------------------------------------------------------------
// process a PROXYC CB update request
//------------------------------------------------------------------------------
hal_ret_t
proxyccb_update (internal::ProxycCbSpec& spec, internal::ProxycCbResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;
    proxyccb_t              *proxyccb;
    pd::pd_proxyccb_update_args_t  pd_proxyccb_args;
    pd::pd_func_args_t          pd_func_args = {0};

    auto kh = spec.key_or_handle();

    proxyccb = find_proxyccb_by_id(kh.proxyccb_id());
    if (proxyccb == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_PROXYC_CB_NOT_FOUND;
    }

    proxyccb->my_txq_base = spec.my_txq_base();
    proxyccb->my_txq_ring_size_shift = spec.my_txq_ring_size_shift();
    proxyccb->my_txq_entry_size_shift = spec.my_txq_entry_size_shift();

    proxyccb->proxyccb_flags = spec.proxyccb_flags();
    proxyccb->chain_txq_base = spec.chain_txq_base();
    proxyccb->chain_txq_ring_indices_addr = spec.chain_txq_ring_indices_addr();
    proxyccb->chain_txq_ring_size_shift = spec.chain_txq_ring_size_shift();
    proxyccb->chain_txq_entry_size_shift = spec.chain_txq_entry_size_shift();
    proxyccb->chain_txq_lif = spec.chain_txq_lif();
    proxyccb->chain_txq_qtype = spec.chain_txq_qtype();
    proxyccb->chain_txq_qid = spec.chain_txq_qid();
    proxyccb->chain_txq_ring = spec.chain_txq_ring();
    proxyccb->redir_span = spec.redir_span();

    pd::pd_proxyccb_update_args_init(&pd_proxyccb_args);
    pd_proxyccb_args.proxyccb = proxyccb;

    pd_func_args.pd_proxyccb_update = &pd_proxyccb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROXYCCB_UPDATE, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD PROXYCCB: Update Failed, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill stats of this PROXYC CB
    rsp->set_api_status(types::API_STATUS_OK);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a PROXYC CB get request
//------------------------------------------------------------------------------
hal_ret_t
proxyccb_get (internal::ProxycCbGetRequest& req, internal::ProxycCbGetResponseMsg *resp)
{
    hal_ret_t               ret = HAL_RET_OK;
    proxyccb_t              rproxyccb;
    proxyccb_t              *proxyccb;
    pd::pd_proxyccb_get_args_t  pd_proxyccb_args;
    pd::pd_func_args_t          pd_func_args = {0};
    ProxycCbGetResponse *rsp = resp->add_response();

    auto kh = req.key_or_handle();

    proxyccb = find_proxyccb_by_id(kh.proxyccb_id());
    if (proxyccb == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_PROXYC_CB_NOT_FOUND;
    }

    proxyccb_init(&rproxyccb);
    rproxyccb.cb_id = proxyccb->cb_id;
    pd::pd_proxyccb_get_args_init(&pd_proxyccb_args);
    pd_proxyccb_args.proxyccb = &rproxyccb;

    pd_func_args.pd_proxyccb_get = &pd_proxyccb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROXYCCB_GET, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD PROXYCCB: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill config spec of this PROXYC CB
    rsp->mutable_spec()->mutable_key_or_handle()->set_proxyccb_id(rproxyccb.cb_id);

    rsp->mutable_spec()->set_proxyccb_flags(rproxyccb.proxyccb_flags);
    rsp->mutable_spec()->set_my_txq_base(rproxyccb.my_txq_base);
    rsp->mutable_spec()->set_my_txq_ring_size_shift(rproxyccb.my_txq_ring_size_shift);
    rsp->mutable_spec()->set_my_txq_entry_size_shift(rproxyccb.my_txq_entry_size_shift);

    rsp->mutable_spec()->set_chain_txq_base(rproxyccb.chain_txq_base);
    rsp->mutable_spec()->set_chain_txq_ring_indices_addr(rproxyccb.chain_txq_ring_indices_addr);
    rsp->mutable_spec()->set_chain_txq_ring_size_shift(rproxyccb.chain_txq_ring_size_shift);
    rsp->mutable_spec()->set_chain_txq_entry_size_shift(rproxyccb.chain_txq_entry_size_shift);
    rsp->mutable_spec()->set_chain_txq_lif(rproxyccb.chain_txq_lif);
    rsp->mutable_spec()->set_chain_txq_qtype(rproxyccb.chain_txq_qtype);
    rsp->mutable_spec()->set_chain_txq_qid(rproxyccb.chain_txq_qid);
    rsp->mutable_spec()->set_chain_txq_ring(rproxyccb.chain_txq_ring);
    rsp->mutable_spec()->set_redir_span(rproxyccb.redir_span);

    rsp->mutable_spec()->set_stat_pkts_chain(rproxyccb.stat_pkts_chain);
    rsp->mutable_spec()->set_stat_pkts_discard(rproxyccb.stat_pkts_discard);
    rsp->mutable_spec()->set_stat_cb_not_ready(rproxyccb.stat_cb_not_ready);
    rsp->mutable_spec()->set_stat_my_txq_empty(rproxyccb.stat_my_txq_empty);
    rsp->mutable_spec()->set_stat_aol_err(rproxyccb.stat_aol_err);
    rsp->mutable_spec()->set_stat_txq_full(rproxyccb.stat_txq_full);
    rsp->mutable_spec()->set_stat_desc_sem_free_full(rproxyccb.stat_desc_sem_free_full);
    rsp->mutable_spec()->set_stat_page_sem_free_full(rproxyccb.stat_page_sem_free_full);

    rsp->mutable_spec()->set_pi(rproxyccb.pi);
    rsp->mutable_spec()->set_ci(rproxyccb.ci);

    // fill operational state of this PROXYC CB
    rsp->mutable_status()->set_proxyccb_handle(proxyccb->hal_handle);

    // fill stats of this PROXYC CB
    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a PROXYC CB delete request
//------------------------------------------------------------------------------
hal_ret_t
proxyccb_delete (internal::ProxycCbDeleteRequest& req, internal::ProxycCbDeleteResponseMsg *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;
    proxyccb_t              *proxyccb;
    pd::pd_proxyccb_args_t  pd_proxyccb_args;
    pd::pd_proxyccb_delete_args_t  del_args;
    pd::pd_func_args_t          pd_func_args = {0};

    auto kh = req.key_or_handle();
    proxyccb = find_proxyccb_by_id(kh.proxyccb_id());
    if (proxyccb == NULL) {
        rsp->add_api_status(types::API_STATUS_OK);
        return HAL_RET_OK;
    }

    pd::pd_proxyccb_delete_args_init(&del_args);
    del_args.r_args = &pd_proxyccb_args;
    pd_proxyccb_args.proxyccb = proxyccb;

    pd_func_args.pd_proxyccb_delete = &del_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROXYCCB_DELETE, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD PROXYCCB: delete Failed, err: {}", ret);
        rsp->add_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    del_proxyccb_from_db(proxyccb);
    proxyccb_free(proxyccb);

    // fill stats of this PROXYC CB
    rsp->add_api_status(types::API_STATUS_OK);

    return HAL_RET_OK;
}

}    // namespace hal
