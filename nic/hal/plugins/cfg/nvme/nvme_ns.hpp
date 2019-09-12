//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NVME_NS_HPP__
#define __NVME_NS_HPP__

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

typedef struct nvme_ns_s {
    sdk_spinlock_t        slock;                   // lock to protect this structure

    uint32_t lif;
    uint32_t cb_id; //LIF local

    uint32_t g_nsid;
    uint32_t lif_nsid;

    uint32_t backend_nsid;
    uint32_t max_sessions;
    uint32_t lba_size;
    uint32_t size; //in LBAs
    uint32_t key_index;
    uint32_t sec_key_index;
    uint32_t g_sess_start;

    void *pd;
    hal_handle_t hal_handle;

    ht_ctxt_t             ht_ctxt;                 // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;      // hal handle based hash table ctxt

} nvme_ns_t;



// max. number of NVME NS CBs supported  (TODO: we can take this from cfg file)
#define HAL_MAX_NVME_NS                           2048

// allocate a nvme_ns instance
static inline nvme_ns_t *
nvme_ns_alloc (void)
{
    nvme_ns_t    *nvme_ns;

    nvme_ns = (nvme_ns_t *)g_hal_state->nvme_ns_slab()->alloc();
    if (nvme_ns == NULL) {
        return NULL;
    }
    return nvme_ns;
}

// initialize a nvme_nsment instance
static inline nvme_ns_t *
nvme_ns_init (nvme_ns_t *nvme_ns)
{
    if (!nvme_ns) {
        return NULL;
    }
    SDK_SPINLOCK_INIT(&nvme_ns->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    nvme_ns->pd = NULL;

    nvme_ns->ht_ctxt.reset();
    nvme_ns->hal_handle_ht_ctxt.reset();

    return nvme_ns;
}

// allocate and initialize a NVME_NS instance
static inline nvme_ns_t *
nvme_ns_alloc_init (void)
{
    return nvme_ns_init(nvme_ns_alloc());
}

static inline hal_ret_t
nvme_ns_free (nvme_ns_t *nvme_ns)
{
    SDK_SPINLOCK_DESTROY(&nvme_ns->slock);
    hal::delay_delete_to_slab(HAL_SLAB_NVME_NS, nvme_ns);
    return HAL_RET_OK;
}

extern void *nvme_ns_get_key_func(void *entry);
extern uint32_t nvme_ns_key_size(void);

extern void *nvme_ns_get_handle_key_func(void *entry);
extern uint32_t nvme_ns_handle_key_size(void);

hal_ret_t
nvme_ns_create(uint32_t lif,
               uint32_t lif_nsid,
               uint32_t g_nsid,
               uint32_t backend_nsid,
               uint32_t max_sessions,
               uint32_t size, //in LBAs
               uint32_t lba_size,
               uint32_t key_index,
               uint32_t sec_key_index,
               uint32_t g_sess_start,
               NvmeNsResponse *rsp);

} //namespace hal

#endif //__NVME_NS_HPP__

