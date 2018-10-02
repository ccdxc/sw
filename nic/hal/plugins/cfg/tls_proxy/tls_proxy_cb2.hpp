//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __TLS_PROXY_CB2_HPP__
#define __TLS_PROXY_CB2_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/encap.hpp"
#include "nic/sdk/include/sdk/list.hpp"
#include "nic/sdk/include/sdk/ht.hpp"
#include "gen/proto/tls_proxy_cb2.pb.h"
#include "nic/include/pd.hpp"
#include "nic/include/hal_state.hpp"

using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;

using tls_proxy::TlsProxyCbSpec;
using tls_proxy::TlsProxyCbStatus;
using tls_proxy::TlsProxyCbResponse;
using tls_proxy::TlsProxyCbKeyHandle;
using tls_proxy::TlsProxyCbRequestMsg;
using tls_proxy::TlsProxyCbResponseMsg;
using tls_proxy::TlsProxyCbDeleteRequestMsg;
using tls_proxy::TlsProxyCbDeleteResponseMsg;
using tls_proxy::TlsProxyCbGetRequest;
using tls_proxy::TlsProxyCbGetRequestMsg;
using tls_proxy::TlsProxyCbGetResponse;
using tls_proxy::TlsProxyCbGetResponseMsg;

namespace hal {

typedef struct tls_proxy_cb_s {
    hal_spinlock_t        slock;                   // lock to protect this structure
    tls_proxy_cb_id_t            cb_id;                   // TLS CB id
    uint32_t              command;
    uint64_t              sesq_base;
    uint64_t              serq_base;
    uint16_t              serq_pi;
    uint16_t              serq_ci;
    uint16_t              bsq_pi;
    uint16_t              bsq_ci;
    uint32_t              crypto_key_idx;
    uint32_t              debug_dol;
    uint64_t              tnmdr_alloc;
    uint64_t              tnmpr_alloc;
    uint64_t              rnmdr_free;
    uint64_t              rnmpr_free;
    uint64_t              enc_requests;
    uint64_t              enc_completions;
    uint64_t              enc_failures;
    uint64_t              dec_requests;
    uint64_t              dec_completions;
    uint64_t              dec_failures;
    uint64_t              mac_requests;
    uint64_t              mac_completions;
    uint64_t              mac_failures;
    uint32_t              salt;
    uint64_t              explicit_iv;
    uint32_t              pre_debug_stage0_7_thread;
    uint32_t              post_debug_stage0_7_thread;
    uint8_t               is_decrypt_flow;
    uint16_t              other_fid;
    types::AppRedirType   l7_proxy_type;
    uint32_t              crypto_hmac_key_idx;
    uint8_t               cpu_id;

    // operational state of TLS Proxy CB
    hal_handle_t          hal_handle;              // HAL allocated handle

    // PD state
    void                  *pd;                     // all PD specific state

    ht_ctxt_t             ht_ctxt;                 // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;      // hal handle based hash table ctxt
} __PACK__ tls_proxy_cb_t;

// max. number of TLS CBs supported  (TODO: we can take this from cfg file)
#define HAL_MAX_TLSCB                           2048

// allocate a tls_proxy_cbment instance
static inline tls_proxy_cb_t *
tls_proxy_cb_alloc (void)
{
    tls_proxy_cb_t    *tls_proxy_cb;

    tls_proxy_cb = (tls_proxy_cb_t *)g_hal_state->tlscb_slab()->alloc();
    if (tls_proxy_cb == NULL) {
        return NULL;
    }
    return tls_proxy_cb;
}

// initialize a tls_proxy_cbment instance
static inline tls_proxy_cb_t *
tls_proxy_cb_init (tls_proxy_cb_t *tls_proxy_cb)
{
    if (!tls_proxy_cb) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&tls_proxy_cb->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    tls_proxy_cb->pd = NULL;

    // initialize meta information
    tls_proxy_cb->ht_ctxt.reset();
    tls_proxy_cb->hal_handle_ht_ctxt.reset();

    return tls_proxy_cb;
}

// allocate and initialize a TLSCB instance
static inline tls_proxy_cb_t *
tls_proxy_cb_alloc_init (void)
{
    return tls_proxy_cb_init(tls_proxy_cb_alloc());
}

static inline hal_ret_t
tls_proxy_cb_free (tls_proxy_cb_t *tls_proxy_cb)
{
    HAL_SPINLOCK_DESTROY(&tls_proxy_cb->slock);
    hal::delay_delete_to_slab(HAL_SLAB_TLSCB, tls_proxy_cb);
    return HAL_RET_OK;
}

static inline tls_proxy_cb_t *
find_tls_proxy_cb_by_id (tls_proxy_cb_id_t tls_proxy_cb_id)
{
    return (tls_proxy_cb_t *)g_hal_state->tlscb_id_ht()->lookup(&tls_proxy_cb_id);
}

extern void *tls_proxy_cb_get_key_func(void *entry);
extern uint32_t tls_proxy_cb_compute_hash_func(void *key, uint32_t ht_size);
extern bool tls_proxy_cb_compare_key_func(void *key1, void *key2);

extern void *tls_proxy_cb_get_handle_key_func(void *entry);
extern uint32_t tls_proxy_cb_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool tls_proxy_cb_compare_handle_key_func(void *key1, void *key2);

hal_ret_t tls_proxy_cb_create(tls_proxy::TlsProxyCbSpec& spec,
                       tls_proxy::TlsProxyCbResponse *rsp);

hal_ret_t tls_proxy_cb_update(tls_proxy::TlsProxyCbSpec& spec,
                       tls_proxy::TlsProxyCbResponse *rsp);

hal_ret_t tls_proxy_cb_delete(tls_proxy::TlsProxyCbDeleteRequest& req,
                       tls_proxy::TlsProxyCbDeleteResponseMsg *rsp);

hal_ret_t tls_proxy_cb_get(tls_proxy::TlsProxyCbGetRequest& req,
                    tls_proxy::TlsProxyCbGetResponseMsg *rsp);
}    // namespace hal

#endif    // __TLS_PROXY_CB2_HPP__

