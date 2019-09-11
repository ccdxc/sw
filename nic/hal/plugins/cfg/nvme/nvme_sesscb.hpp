//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NVME_SESSCB_HPP__
#define __NVME_SESSCB_HPP__

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

typedef struct nvme_sesscb_s {
    sdk_spinlock_t        slock;                   // lock to protect this structure

    uint32_t lif;
    uint32_t g_nsid;
    uint32_t cb_id; //LIF local

    uint32_t g_sess_id;
    uint32_t lif_sess_id;
    uint32_t ns_sess_id;

    uint32_t serq_qid;
    uint32_t sesq_qid;

    void *pd;
    hal_handle_t hal_handle;

    ht_ctxt_t             ht_ctxt;                 // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;      // hal handle based hash table ctxt

} nvme_sesscb_t;



// max. number of NVME SESS CBs supported  (TODO: we can take this from cfg file)
#define HAL_MAX_NVME_SESSCB                           2048

// allocate a nvme_sesscbment instance
static inline nvme_sesscb_t *
nvme_sesscb_alloc (void)
{
    nvme_sesscb_t    *nvme_sesscb;

    nvme_sesscb = (nvme_sesscb_t *)g_hal_state->nvme_sesscb_slab()->alloc();
    if (nvme_sesscb == NULL) {
        return NULL;
    }
    return nvme_sesscb;
}

// initialize a nvme_sesscbment instance
static inline nvme_sesscb_t *
nvme_sesscb_init (nvme_sesscb_t *nvme_sesscb)
{
    if (!nvme_sesscb) {
        return NULL;
    }
    SDK_SPINLOCK_INIT(&nvme_sesscb->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    nvme_sesscb->pd = NULL;

    nvme_sesscb->ht_ctxt.reset();
    nvme_sesscb->hal_handle_ht_ctxt.reset();

    return nvme_sesscb;
}

// allocate and initialize a NVME_SESSCB instance
static inline nvme_sesscb_t *
nvme_sesscb_alloc_init (void)
{
    return nvme_sesscb_init(nvme_sesscb_alloc());
}

static inline hal_ret_t
nvme_sesscb_free (nvme_sesscb_t *nvme_sesscb)
{
    SDK_SPINLOCK_DESTROY(&nvme_sesscb->slock);
    hal::delay_delete_to_slab(HAL_SLAB_NVME_SESSCB, nvme_sesscb);
    return HAL_RET_OK;
}

extern void *nvme_sesscb_get_key_func(void *entry);
extern uint32_t nvme_sesscb_compute_hash_func(void *key, uint32_t ht_size);
extern bool nvme_sesscb_compare_key_func(void *key1, void *key2);

extern void *nvme_sesscb_get_handle_key_func(void *entry);
extern uint32_t nvme_sesscb_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool nvme_sesscb_compare_handle_key_func(void *key1, void *key2);


hal_ret_t nvme_sesscb_create(uint32_t lif, uint32_t g_nsid, uint32_t g_sess_id, uint32_t lif_sess_id,
                             uint32_t ns_sess_id, uint32_t sesq_qid, uint32_t serq_qid, NvmeSessResponse *rsp);


} //namespace hal

#endif //__NVME_SESSCB_HPP__

