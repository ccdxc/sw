//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NVME_CQ_HPP__
#define __NVME_CQ_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/encap.hpp"
#include "lib/list/list.hpp"
#include "lib/ht/ht.hpp"
#include "gen/proto/internal.pb.h"
#include "nic/include/pd.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/nvme/nvme.hpp"


using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;

namespace hal {

typedef struct nvme_cq_s {
    sdk_spinlock_t        slock;                   // lock to protect this structure

    uint32_t              lif;
    uint32_t              cb_id; //LIF local

    uint32_t              cq_id;
    uint32_t              log_wqe_size;
    uint32_t              log_num_wqes;
    uint32_t              int_num;
    uint64_t              base_addr;

    void                  *pd;
    hal_handle_t          hal_handle;

    ht_ctxt_t             ht_ctxt;                 // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;      // hal handle based hash table ctxt

} nvme_cq_t;


// max. number of NVME NS CBs supported  (TODO: we can take this from cfg file)
#define HAL_MAX_NVME_CQ                           (1024)

// allocate a nvme_cq instance
static inline nvme_cq_t *
nvme_cq_alloc (void)
{
    nvme_cq_t    *nvme_cq;

    nvme_cq = (nvme_cq_t *)g_hal_state->nvme_cq_slab()->alloc();
    if (nvme_cq == NULL) {
        return NULL;
    }
    return nvme_cq;
}

// initialize a nvme_cqment instance
static inline nvme_cq_t *
nvme_cq_init (nvme_cq_t *nvme_cq)
{
    if (!nvme_cq) {
        return NULL;
    }
    SDK_SPINLOCK_INIT(&nvme_cq->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    nvme_cq->pd = NULL;

    nvme_cq->ht_ctxt.reset();
    nvme_cq->hal_handle_ht_ctxt.reset();

    return nvme_cq;
}

// allocate and initialize a NVME_CQ instance
static inline nvme_cq_t *
nvme_cq_alloc_init (void)
{
    return nvme_cq_init(nvme_cq_alloc());
}

static inline hal_ret_t
nvme_cq_free (nvme_cq_t *nvme_cq)
{
    SDK_SPINLOCK_DESTROY(&nvme_cq->slock);
    hal::delay_delete_to_slab(HAL_SLAB_NVME_CQ, nvme_cq);
    return HAL_RET_OK;
}

extern void *nvme_cq_get_key_func(void *entry);
extern uint32_t nvme_cq_compute_hash_func(void *key, uint32_t ht_size);
extern bool nvme_cq_compare_key_func(void *key1, void *key2);

extern void *nvme_cq_get_handle_key_func(void *entry);
extern uint32_t nvme_cq_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool nvme_cq_compare_handle_key_func(void *key1, void *key2);

hal_ret_t
nvme_cq_create(uint32_t lif,
               uint32_t cq_id,
               uint32_t log_wqe_size,
               uint32_t log_num_wqes,
               uint32_t int_num,
               uint64_t base_addr,
               NvmeCqResponse *rsp);

} //namespace hal

#endif //__NVME_CQ_HPP__

