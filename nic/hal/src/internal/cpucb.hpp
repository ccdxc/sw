//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __CPUCB_HPP__
#define __CPUCB_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/encap.hpp"
#include "lib/list/list.hpp"
#include "lib/ht/ht.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/lif/lif.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#ifdef __x86_64__
#include "gen/proto/cpucb.pb.h"
#endif

#define MAX_CPU_CBID 8

using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;

#ifdef __x86_64__
using cpucb::CpuCbSpec;
using cpucb::CpuCbStatus;
using cpucb::CpuCbResponse;
using cpucb::CpuCbKeyHandle;
using cpucb::CpuCbRequestMsg;
using cpucb::CpuCbResponseMsg;
using cpucb::CpuCbDeleteRequestMsg;
using cpucb::CpuCbDeleteResponseMsg;
using cpucb::CpuCbGetRequest;
using cpucb::CpuCbGetRequestMsg;
using cpucb::CpuCbGetResponse;
using cpucb::CpuCbGetResponseMsg;
#endif

namespace hal {

typedef struct cpucb_s {
    sdk_spinlock_t        slock;                   // lock to protect this structure
    cpucb_id_t            cb_id;                   // CB id
    uint8_t               cfg_flags;               // Configuration flags
    hal_handle_t          hal_handle;              // HAL allocated handle
    uint32_t              debug_dol;               // Debug via dol
    uint64_t              descr_addr_oob_count;
    uint64_t              total_tx_pkts;
    uint64_t              total_rx_pkts;
    uint64_t              rx_qfull_drop_errors;
    uint64_t              tx_sem_full_drops;
    uint64_t              ascq_free_requests;
    uint64_t              rx_sem_full_drops;
    uint64_t              rx_queue0_pkts;
    uint64_t              rx_queue1_pkts;
    uint64_t              rx_queue2_pkts;

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
    SDK_SPINLOCK_INIT(&cpucb->slock, PTHREAD_PROCESS_PRIVATE);

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
    SDK_SPINLOCK_DESTROY(&cpucb->slock);
    hal::delay_delete_to_slab(HAL_SLAB_CPUCB, cpucb);
    return HAL_RET_OK;
}

static inline cpucb_t *
find_cpucb_by_id (cpucb_id_t cpucb_id)
{
    return (cpucb_t *)g_hal_state->cpucb_id_ht()->lookup(&cpucb_id);
}

extern void *cpucb_get_key_func(void *entry);
extern uint32_t cpucb_key_size(void);

extern void *cpucb_get_handle_key_func(void *entry);
extern uint32_t cpucb_handle_key_size(void);

hal_ret_t cpucb_get_by_id (cpucb_id_t cpucb_id, cpucb_t &cpucb, lif_id_t lif_id = HAL_LIF_CPU);

#ifdef __x86_64__
hal_ret_t
cpucb_get_stats ( lif_id_t lif_id, LifGetResponse *rsp);

hal_ret_t cpucb_create(cpucb::CpuCbSpec& spec,
                       cpucb::CpuCbResponse *rsp);

hal_ret_t cpucb_update(cpucb::CpuCbSpec& spec,
                       cpucb::CpuCbResponse *rsp);

hal_ret_t cpucb_delete(cpucb::CpuCbDeleteRequest& req,
                       cpucb::CpuCbDeleteResponse *rsp);

hal_ret_t cpucb_get(cpucb::CpuCbGetRequest& req,
                    cpucb::CpuCbGetResponseMsg *rsp);
#endif

}    // namespace hal

#endif    // __CPUCB_HPP__

