#ifndef __WRING_HPP__
#define __WRING_HPP__

#include <base.h>
#include <encap.hpp>
#include <list.hpp>
#include <ht.hpp>
#include <wring.pb.h>
#include <pd.hpp>
#include <hal_state.hpp>

using hal::utils::ht_ctxt_t;
using hal::utils::dllist_ctxt_t;

namespace hal {

typedef uint32_t wring_id_t;

typedef struct wring_s {
    hal_spinlock_t        slock;                   // lock to protect this structure
    wring_id_t            wring_id;                // WRing id
    types::WRingType      wring_type;              // Wring Type
    uint64_t              slot_index;              // PI/CI for the request
    uint64_t              slot_value;              // Slot Value
    // operational state of WRing
    hal_handle_t          hal_handle;              // HAL allocated handle

    // PD state
    void                  *pd;                     // all PD specific state

    ht_ctxt_t             ht_ctxt;                 // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;      // hal handle based hash table ctxt
} __PACK__ wring_t;

// max. number of WRING supported  (TODO: we can take this from cfg file)
#define HAL_MAX_WRING                           2048

// allocate a wringment instance
static inline wring_t *
wring_alloc (void)
{
    wring_t    *wring;

    wring = (wring_t *)g_hal_state->wring_slab()->alloc();
    if (wring == NULL) {
        return NULL;
    }
    return wring;
}

// initialize a wringment instance
static inline wring_t *
wring_init (wring_t *wring)
{
    if (!wring) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&wring->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    wring->pd = NULL;

    // initialize meta information
    wring->ht_ctxt.reset();
    wring->hal_handle_ht_ctxt.reset();

    return wring;
}

// allocate and initialize a WRING instance
static inline wring_t *
wring_alloc_init (void)
{
    return wring_init(wring_alloc());
}

static inline hal_ret_t
wring_free (wring_t *wring)
{
    HAL_SPINLOCK_DESTROY(&wring->slock);
    g_hal_state->wring_slab()->free(wring);
    return HAL_RET_OK;
}

static inline wring_t *
find_wring_by_id (wring_id_t wring_id)
{
    return (wring_t *)g_hal_state->wring_id_ht()->lookup(&wring_id);
}

static inline wring_t *
find_wring_by_handle (hal_handle_t handle)
{
    return (wring_t *)g_hal_state->wring_hal_handle_ht()->lookup(&handle);
}

extern void *wring_get_key_func(void *entry);
extern uint32_t wring_compute_hash_func(void *key, uint32_t ht_size);
extern bool wring_compare_key_func(void *key1, void *key2);

extern void *wring_get_handle_key_func(void *entry);
extern uint32_t wring_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool wring_compare_handle_key_func(void *key1, void *key2);

hal_ret_t wring_create(wring::WRingSpec& spec,
                       wring::WRingResponse *rsp);

hal_ret_t wring_update(wring::WRingSpec& spec,
                       wring::WRingResponse *rsp);

hal_ret_t wring_get(wring::WRingGetRequest& req,
                    wring::WRingGetResponse *rsp);
}    // namespace hal

#endif    // __WRING_HPP__

