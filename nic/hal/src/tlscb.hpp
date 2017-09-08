#ifndef __TLSCB_HPP__
#define __TLSCB_HPP__

#include <base.h>
#include <encap.hpp>
#include <list.hpp>
#include <ht.hpp>
#include <tls_proxy_cb.pb.h>
#include <pd.hpp>
#include <hal_state.hpp>

using hal::utils::ht_ctxt_t;
using hal::utils::dllist_ctxt_t;

namespace hal {

typedef uint32_t tlscb_id_t;

typedef struct tlscb_s {
    hal_spinlock_t        slock;                   // lock to protect this structure
    tlscb_id_t            cb_id;                   // TLS CB id
    uint8_t               cipher_type;
    uint64_t              sesq_base;
    uint64_t              serq_base;    
    uint16_t              serq_pi;
    uint16_t              serq_ci;
    uint16_t              bsq_pi;
    uint16_t              bsq_ci;
    uint32_t              crypto_key_idx;
    uint32_t              debug_dol;    

    // operational state of TLS Proxy CB
    hal_handle_t          hal_handle;              // HAL allocated handle

    // PD state
    void                  *pd;                     // all PD specific state

    ht_ctxt_t             ht_ctxt;                 // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;      // hal handle based hash table ctxt
} __PACK__ tlscb_t;

// max. number of TLS CBs supported  (TODO: we can take this from cfg file)
#define HAL_MAX_TLSCB                           2048

// allocate a tlscbment instance
static inline tlscb_t *
tlscb_alloc (void)
{
    tlscb_t    *tlscb;

    tlscb = (tlscb_t *)g_hal_state->tlscb_slab()->alloc();
    if (tlscb == NULL) {
        return NULL;
    }
    return tlscb;
}

// initialize a tlscbment instance
static inline tlscb_t *
tlscb_init (tlscb_t *tlscb)
{
    if (!tlscb) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&tlscb->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    tlscb->pd = NULL;

    // initialize meta information
    tlscb->ht_ctxt.reset();
    tlscb->hal_handle_ht_ctxt.reset();

    return tlscb;
}

// allocate and initialize a TLSCB instance
static inline tlscb_t *
tlscb_alloc_init (void)
{
    return tlscb_init(tlscb_alloc());
}

static inline hal_ret_t
tlscb_free (tlscb_t *tlscb)
{
    HAL_SPINLOCK_DESTROY(&tlscb->slock);
    g_hal_state->tlscb_slab()->free(tlscb);
    return HAL_RET_OK;
}

static inline tlscb_t *
find_tlscb_by_id (tlscb_id_t tlscb_id)
{
    return (tlscb_t *)g_hal_state->tlscb_id_ht()->lookup(&tlscb_id);
}

static inline tlscb_t *
find_tlscb_by_handle (hal_handle_t handle)
{
    return (tlscb_t *)g_hal_state->tlscb_hal_handle_ht()->lookup(&handle);
}

extern void *tlscb_get_key_func(void *entry);
extern uint32_t tlscb_compute_hash_func(void *key, uint32_t ht_size);
extern bool tlscb_compare_key_func(void *key1, void *key2);

extern void *tlscb_get_handle_key_func(void *entry);
extern uint32_t tlscb_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool tlscb_compare_handle_key_func(void *key1, void *key2);

hal_ret_t tlscb_create(tlscb::TlsCbSpec& spec,
                       tlscb::TlsCbResponse *rsp);

hal_ret_t tlscb_update(tlscb::TlsCbSpec& spec,
                       tlscb::TlsCbResponse *rsp);

hal_ret_t tlscb_delete(tlscb::TlsCbDeleteRequest& req,
                       tlscb::TlsCbDeleteResponseMsg *rsp);

hal_ret_t tlscb_get(tlscb::TlsCbGetRequest& req,
                    tlscb::TlsCbGetResponse *rsp);
}    // namespace hal

#endif    // __TLSCB_HPP__

