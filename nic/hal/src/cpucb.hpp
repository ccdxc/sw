#ifndef __CPUCB_HPP__
#define __CPUCB_HPP__

#include <base.h>
#include <encap.hpp>
#include <list.hpp>
#include <ht.hpp>
#include <cpucb.pb.h>
#include <pd.hpp>
#include <hal_state.hpp>

using hal::utils::ht_ctxt_t;
using hal::utils::dllist_ctxt_t;

namespace hal {

typedef uint32_t cpucb_id_t;

typedef struct cpucb_s {
    hal_spinlock_t        slock;                   // lock to protect this structure
    cpucb_id_t            cb_id;                   // CB id
    hal_handle_t          hal_handle;              // HAL allocated handle

    // PD state
    void                  *pd;                     // all PD specific state

    ht_ctxt_t             ht_ctxt;                 // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;      // hal handle based hash table ctxt
} __PACK__ cpucb_t;

// max. number of CBs supported  (TODO: we can take this from cfg file)
#define HAL_MAX_CPUCB                           4

// allocate a cpucbment instance
static inline cpucb_t *
cpucb_alloc (void)
{
    cpucb_t    *cpucb;

    cpucb = (cpucb_t *)g_hal_state->cpucb_slab()->alloc();
    if (cpucb == NULL) {
        return NULL;
    }
    return cpucb;
}

// initialize a cpucbment instance
static inline cpucb_t *
cpucb_init (cpucb_t *cpucb)
{
    if (!cpucb) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&cpucb->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    cpucb->pd = NULL;

    // initialize meta information
    cpucb->ht_ctxt.reset();
    cpucb->hal_handle_ht_ctxt.reset();

    return cpucb;
}

// allocate and initialize a CPUCB instance
static inline cpucb_t *
cpucb_alloc_init (void)
{
    return cpucb_init(cpucb_alloc());
}

static inline hal_ret_t
cpucb_free (cpucb_t *cpucb)
{
    HAL_SPINLOCK_DESTROY(&cpucb->slock);
    g_hal_state->cpucb_slab()->free(cpucb);
    return HAL_RET_OK;
}

static inline cpucb_t *
find_cpucb_by_id (cpucb_id_t cpucb_id)
{
    return (cpucb_t *)g_hal_state->cpucb_id_ht()->lookup(&cpucb_id);
}

static inline cpucb_t *
find_cpucb_by_handle (hal_handle_t handle)
{
    return (cpucb_t *)g_hal_state->cpucb_hal_handle_ht()->lookup(&handle);
}

extern void *cpucb_get_key_func(void *entry);
extern uint32_t cpucb_compute_hash_func(void *key, uint32_t ht_size);
extern bool cpucb_compare_key_func(void *key1, void *key2);

extern void *cpucb_get_handle_key_func(void *entry);
extern uint32_t cpucb_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool cpucb_compare_handle_key_func(void *key1, void *key2);

hal_ret_t cpucb_create(cpucb::CpuCbSpec& spec,
                       cpucb::CpuCbResponse *rsp);

hal_ret_t cpucb_update(cpucb::CpuCbSpec& spec,
                       cpucb::CpuCbResponse *rsp);

hal_ret_t cpucb_delete(cpucb::CpuCbDeleteRequest& req,
                       cpucb::CpuCbDeleteResponseMsg *rsp);

hal_ret_t cpucb_get(cpucb::CpuCbGetRequest& req,
                    cpucb::CpuCbGetResponse *rsp);
}    // namespace hal

#endif    // __CPUCB_HPP__

