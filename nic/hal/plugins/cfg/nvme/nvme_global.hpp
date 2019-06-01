//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NVME_GLOBAL_HPP__
#define __NVME_GLOBAL_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/encap.hpp"
#include "lib/list/list.hpp"
#include "lib/ht/ht.hpp"
#include "gen/proto/internal.pb.h"
#include "nic/include/pd.hpp"
#include "nic/hal/iris/include/hal_state.hpp"

using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;

namespace hal {

typedef struct nvme_global_s {
    sdk_spinlock_t        slock;                   // lock to protect this structure

    uint32_t max_lif;
    uint32_t max_ns;
    uint32_t max_sess;
    uint32_t max_cmd_context;
    uint32_t tx_max_pdu_context;
    uint32_t rx_max_pdu_context;

    void *pd;
    hal_handle_t hal_handle;

} nvme_global_t;


// allocate a nvme_global instance
static inline nvme_global_t *
nvme_global_alloc (void)
{
    nvme_global_t    *nvme_global;

    nvme_global = (nvme_global_t *)g_hal_state->nvme_global_slab()->alloc();
    if (nvme_global == NULL) {
        return NULL;
    }
    return nvme_global;
}

// initialize a nvme_global instance
static inline nvme_global_t *
nvme_global_init (nvme_global_t *nvme_global)
{
    if (!nvme_global) {
        return NULL;
    }
    SDK_SPINLOCK_INIT(&nvme_global->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    nvme_global->pd = NULL;

    return nvme_global;
}

// allocate and initialize a NVME_GLOBAL instance
static inline nvme_global_t *
nvme_global_alloc_init (void)
{
    return nvme_global_init(nvme_global_alloc());
}

static inline hal_ret_t
nvme_global_free (nvme_global_t *nvme_global)
{
    SDK_SPINLOCK_DESTROY(&nvme_global->slock);
    hal::delay_delete_to_slab(HAL_SLAB_NVME_GLOBAL, nvme_global);
    return HAL_RET_OK;
}

hal_ret_t nvme_global_create(uint32_t max_lif, uint32_t max_ns, uint32_t max_sess,
                             uint32_t max_cmd_context, uint32_t tx_max_pdu_context, 
                             uint32_t rx_max_pdu_context);


} //namespace hal

#endif //__NVME_GLOBAL_HPP__

