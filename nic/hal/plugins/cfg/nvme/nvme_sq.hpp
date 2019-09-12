//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NVME_SQ_HPP__
#define __NVME_SQ_HPP__

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

typedef struct nvme_sq_s {
    sdk_spinlock_t        slock;                   // lock to protect this structure

    uint32_t              lif;
    uint32_t              cb_id; //LIF local

    uint32_t              sq_id;
    uint32_t              log_wqe_size;
    uint32_t              log_num_wqes;
    uint32_t              log_host_page_size;
    uint32_t              cq_id;
    uint32_t              ns_start;
    uint64_t              base_addr;

    void                  *pd;
    hal_handle_t          hal_handle;

    ht_ctxt_t             ht_ctxt;                 // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;      // hal handle based hash table ctxt

} nvme_sq_t;


// max. number of NVME NS CBs supported  (TODO: we can take this from cfg file)
#define HAL_MAX_NVME_SQ                           (64 * 1024)

// allocate a nvme_sq instance
static inline nvme_sq_t *
nvme_sq_alloc (void)
{
    nvme_sq_t    *nvme_sq;

    nvme_sq = (nvme_sq_t *)g_hal_state->nvme_sq_slab()->alloc();
    if (nvme_sq == NULL) {
        return NULL;
    }
    return nvme_sq;
}

// initialize a nvme_sqment instance
static inline nvme_sq_t *
nvme_sq_init (nvme_sq_t *nvme_sq)
{
    if (!nvme_sq) {
        return NULL;
    }
    SDK_SPINLOCK_INIT(&nvme_sq->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    nvme_sq->pd = NULL;

    nvme_sq->ht_ctxt.reset();
    nvme_sq->hal_handle_ht_ctxt.reset();

    return nvme_sq;
}

// allocate and initialize a NVME_SQ instance
static inline nvme_sq_t *
nvme_sq_alloc_init (void)
{
    return nvme_sq_init(nvme_sq_alloc());
}

static inline hal_ret_t
nvme_sq_free (nvme_sq_t *nvme_sq)
{
    SDK_SPINLOCK_DESTROY(&nvme_sq->slock);
    hal::delay_delete_to_slab(HAL_SLAB_NVME_SQ, nvme_sq);
    return HAL_RET_OK;
}

extern void *nvme_sq_get_key_func(void *entry);
extern uint32_t nvme_sq_key_size(void);

extern void *nvme_sq_get_handle_key_func(void *entry);
extern uint32_t nvme_sq_handle_key_size(void);

hal_ret_t
nvme_sq_create(uint32_t lif,
               uint32_t sq_id,
               uint32_t log_wqe_size,
               uint32_t log_num_wqes,
               uint32_t log_host_page_size,
               uint32_t cq_id,
               uint32_t ns_start,
               uint64_t base_addr,
               NvmeSqResponse *rsp);

} //namespace hal

#endif //__NVME_SQ_HPP__

