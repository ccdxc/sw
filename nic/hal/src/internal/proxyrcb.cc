//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/src/internal/proxyrcb.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/app_redir_shared.h"

namespace hal {
void *
proxyrcb_get_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((proxyrcb_t *)entry)->cb_id);
}

uint32_t
proxyrcb_key_size ()
{
    return sizeof(proxyrcb_id_t);
}

void *
proxyrcb_get_handle_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((proxyrcb_t *)entry)->hal_handle);
}

uint32_t
proxyrcb_handle_key_size ()
{
    return sizeof(hal_handle_t);
}

// allocate a PROXYRCB instance
static inline proxyrcb_t *
proxyrcb_alloc (void)
{
    proxyrcb_t    *proxyrcb;

    proxyrcb = (proxyrcb_t *)g_hal_state->proxyrcb_slab()->alloc();
    if (proxyrcb == NULL) {
        return NULL;
    }
    return proxyrcb;
}

// initialize a PROXYRCB instance
static inline proxyrcb_t *
proxyrcb_init (proxyrcb_t *proxyrcb)
{
    if (!proxyrcb) {
        return NULL;
    }
    SDK_SPINLOCK_INIT(&proxyrcb->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    proxyrcb->pd = NULL;

    // initialize meta information
    proxyrcb->ht_ctxt.reset();
    proxyrcb->hal_handle_ht_ctxt.reset();

    return proxyrcb;
}

// allocate and initialize a PROXYRCB instance
static inline proxyrcb_t *
proxyrcb_alloc_init (void)
{
    return proxyrcb_init(proxyrcb_alloc());
}

static inline hal_ret_t
proxyrcb_free (proxyrcb_t *proxyrcb)
{
    SDK_SPINLOCK_DESTROY(&proxyrcb->slock);
    hal::delay_delete_to_slab(HAL_SLAB_PROXYRCB, proxyrcb);
    return HAL_RET_OK;
}

proxyrcb_t *
find_proxyrcb_by_id (proxyrcb_id_t proxyrcb_id)
{
    return (proxyrcb_t *)g_hal_state->proxyrcb_id_ht()->lookup(&proxyrcb_id);
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
    if (spec.key_or_handle().proxyrcb_id() >= PROXYRCB_NUM_ENTRIES_MAX) {
        rsp->set_api_status(types::API_STATUS_PROXYR_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert/delete this PROXYR CB in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_proxyrcb_to_db (proxyrcb_t *proxyrcb)
{
    g_hal_state->proxyrcb_id_ht()->insert(proxyrcb, &proxyrcb->ht_ctxt);
    return HAL_RET_OK;
}

static inline void
del_proxyrcb_from_db (proxyrcb_t *proxyrcb)
{
    g_hal_state->proxyrcb_id_ht()->remove_entry(proxyrcb, &proxyrcb->ht_ctxt);
}

//------------------------------------------------------------------------------
// process a PROXYR CB create request
// TODO: if PROXYR CB exists, treat this as modify (vrf id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
proxyrcb_create (internal::ProxyrCbSpec& spec, internal::ProxyrCbResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;
    proxyrcb_t              *proxyrcb = NULL;
    pd::pd_proxyrcb_create_args_t  pd_proxyrcb_args;
    pd::pd_func_args_t          pd_func_args = {0};

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
    proxyrcb->dir = spec.dir();
    proxyrcb->role = spec.role();
    proxyrcb->rev_cb_id = spec.rev_cb_id();
    proxyrcb->redir_span = spec.redir_span();

    // allocate all PD resources and finish programming
    pd::pd_proxyrcb_create_args_init(&pd_proxyrcb_args);
    pd_proxyrcb_args.proxyrcb = proxyrcb;
    pd_func_args.pd_proxyrcb_create = &pd_proxyrcb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROXYRCB_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD PROXYR CB create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // add this PROXYR CB to our db
    ret = add_proxyrcb_to_db(proxyrcb);
    SDK_ASSERT(ret == HAL_RET_OK);

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
proxyrcb_update (internal::ProxyrCbSpec& spec, internal::ProxyrCbResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;
    proxyrcb_t              *proxyrcb;
    pd::pd_proxyrcb_update_args_t  pd_proxyrcb_args;
    pd::pd_func_args_t          pd_func_args = {0};

    auto kh = spec.key_or_handle();

    proxyrcb = find_proxyrcb_by_id(kh.proxyrcb_id());
    if (proxyrcb == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
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
    proxyrcb->redir_span = spec.redir_span();

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

    pd::pd_proxyrcb_update_args_init(&pd_proxyrcb_args);
    pd_proxyrcb_args.proxyrcb = proxyrcb;

    pd_func_args.pd_proxyrcb_update = &pd_proxyrcb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROXYRCB_UPDATE, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD PROXYRCB: Update Failed, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
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
proxyrcb_get (internal::ProxyrCbGetRequest& req, internal::ProxyrCbGetResponseMsg *resp)
{
    hal_ret_t               ret = HAL_RET_OK;
    proxyrcb_t              rproxyrcb;
    proxyrcb_t              *proxyrcb;
    pd::pd_proxyrcb_get_args_t  pd_proxyrcb_args;
    pd::pd_func_args_t          pd_func_args = {0};
    ProxyrCbGetResponse *rsp = resp->add_response();

    auto kh = req.key_or_handle();

    proxyrcb = find_proxyrcb_by_id(kh.proxyrcb_id());
    if (proxyrcb == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_PROXYR_CB_NOT_FOUND;
    }

    proxyrcb_init(&rproxyrcb);
    rproxyrcb.cb_id = proxyrcb->cb_id;
    pd::pd_proxyrcb_get_args_init(&pd_proxyrcb_args);
    pd_proxyrcb_args.proxyrcb = &rproxyrcb;

    pd_func_args.pd_proxyrcb_get = &pd_proxyrcb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROXYRCB_GET, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD PROXYRCB: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
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
    rsp->mutable_spec()->set_redir_span(rproxyrcb.redir_span);

    if (rproxyrcb.af == AF_INET) {
        rsp->mutable_spec()->mutable_ip_sa()->set_ip_af(types::IP_AF_INET);
        rsp->mutable_spec()->mutable_ip_sa()->set_v4_addr(rproxyrcb.ip_sa.v4_addr);
        rsp->mutable_spec()->mutable_ip_da()->set_ip_af(types::IP_AF_INET);
        rsp->mutable_spec()->mutable_ip_da()->set_v4_addr(rproxyrcb.ip_da.v4_addr);
    } else {
        rsp->mutable_spec()->mutable_ip_sa()->set_ip_af(types::IP_AF_INET6);
        rsp->mutable_spec()->mutable_ip_sa()->set_v6_addr(&rproxyrcb.ip_sa.v6_addr,
                                                          IP6_ADDR8_LEN);
        rsp->mutable_spec()->mutable_ip_da()->set_ip_af(types::IP_AF_INET6);
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

    rsp->mutable_spec()->set_dir(rproxyrcb.dir);
    rsp->mutable_spec()->set_role(rproxyrcb.role);
    rsp->mutable_spec()->set_rev_cb_id(rproxyrcb.rev_cb_id);

    rsp->mutable_spec()->set_stat_pkts_redir(rproxyrcb.stat_pkts_redir);
    rsp->mutable_spec()->set_stat_pkts_discard(rproxyrcb.stat_pkts_discard);
    rsp->mutable_spec()->set_stat_cb_not_ready(rproxyrcb.stat_cb_not_ready);
    rsp->mutable_spec()->set_stat_null_ring_indices_addr(rproxyrcb.stat_null_ring_indices_addr);
    rsp->mutable_spec()->set_stat_aol_err(rproxyrcb.stat_aol_err);
    rsp->mutable_spec()->set_stat_rxq_full(rproxyrcb.stat_rxq_full);
    rsp->mutable_spec()->set_stat_txq_empty(rproxyrcb.stat_txq_empty);
    rsp->mutable_spec()->set_stat_sem_alloc_full(rproxyrcb.stat_sem_alloc_full);
    rsp->mutable_spec()->set_stat_sem_free_full(rproxyrcb.stat_sem_free_full);

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
proxyrcb_delete (internal::ProxyrCbDeleteRequest& req, internal::ProxyrCbDeleteResponseMsg *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;
    proxyrcb_t              *proxyrcb;
    pd::pd_proxyrcb_args_t  pd_proxyrcb_args;
    pd::pd_proxyrcb_delete_args_t  del_args;
    pd::pd_func_args_t          pd_func_args = {0};

    auto kh = req.key_or_handle();
    proxyrcb = find_proxyrcb_by_id(kh.proxyrcb_id());
    if (proxyrcb == NULL) {
        rsp->add_api_status(types::API_STATUS_OK);
        return HAL_RET_OK;
    }

    pd::pd_proxyrcb_delete_args_init(&del_args);
    del_args.r_args = &pd_proxyrcb_args;
    pd_proxyrcb_args.proxyrcb = proxyrcb;

    pd_func_args.pd_proxyrcb_delete = &del_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROXYRCB_DELETE, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD PROXYRCB: delete Failed, err: {}", ret);
        rsp->add_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    del_proxyrcb_from_db(proxyrcb);
    proxyrcb_free(proxyrcb);

    // fill stats of this PROXYR CB
    rsp->add_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

}    // namespace hal
