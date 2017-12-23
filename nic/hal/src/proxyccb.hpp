#ifndef __PROXYCCB_HPP__
#define __PROXYCCB_HPP__

#include "nic/include/base.h"
#include "nic/include/encap.hpp"
#include "nic/include/list.hpp"
#include "sdk/ht.hpp"
#include "nic/gen/proto/hal/proxyccb.pb.h"
#include "nic/include/pd.hpp"
#include "nic/include/hal_state.hpp"

using sdk::lib::ht_ctxt_t;
using hal::utils::dllist_ctxt_t;

using proxyccb::ProxycCbSpec;
using proxyccb::ProxycCbStatus;
using proxyccb::ProxycCbResponse;
using proxyccb::ProxycCbKeyHandle;
using proxyccb::ProxycCbRequestMsg;
using proxyccb::ProxycCbResponseMsg;
using proxyccb::ProxycCbDeleteRequestMsg;
using proxyccb::ProxycCbDeleteResponseMsg;
using proxyccb::ProxycCbGetRequest;
using proxyccb::ProxycCbGetRequestMsg;
using proxyccb::ProxycCbGetResponse;
using proxyccb::ProxycCbGetResponseMsg;

namespace hal {

typedef struct proxyccb_s {
    hal_spinlock_t        slock;                   // lock to protect this structure
    proxyccb_id_t         cb_id;

    /*
     * Note that ordering of fields below does not matter;
     * data will get written to HBM according to P4+ table entry's
     * ordering defined in proxyc_txdma_p4plus_ingress.h
     * see hal/pd/iris/proxyccb_pd.cc)
     */
    uint16_t              pi;                       // if more PI/CI's are added, 
    uint16_t              ci;                       // be sure to adjust HAL_PROXYCCB_NUM_RINGS_MAX below
    uint16_t              proxyccb_flags;
    uint8_t               my_txq_ring_size_shift;
    uint8_t               my_txq_entry_size_shift;
    uint64_t              my_txq_base;

    uint64_t              chain_txq_base;           // next service chain TxQ base
    uint64_t              chain_txq_ring_indices_addr;
    uint32_t              chain_txq_qid;
    uint16_t              chain_txq_lif;
    uint8_t               chain_txq_qtype;
    uint8_t               chain_txq_ring_size_shift;
    uint8_t               chain_txq_entry_size_shift;
    uint8_t               chain_txq_ring;

    hal_handle_t          hal_handle;               // HAL allocated handle

    // PD state
    void                  *pd;                      // all PD specific state

    ht_ctxt_t             ht_ctxt;                  // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;       // hal handle based hash table ctxt
} __PACK__ proxyccb_t;

#define HAL_MAX_PROXYCCB_HT_SIZE          1024      // hash table size

/*
 * Number of PI/CI pairs defined in proxyccb_t above
 */
#define HAL_NUM_PROXYCCB_RINGS_MAX        1

// allocate a PROXYCCB instance
static inline proxyccb_t *
proxyccb_alloc (void)
{
    proxyccb_t    *proxyccb;

    proxyccb = (proxyccb_t *)g_hal_state->proxyccb_slab()->alloc();
    if (proxyccb == NULL) {
        return NULL;
    }
    return proxyccb;
}

// initialize a PROXYCCB instance
static inline proxyccb_t *
proxyccb_init (proxyccb_t *proxyccb)
{
    if (!proxyccb) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&proxyccb->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    proxyccb->pd = NULL;

    // initialize meta information
    proxyccb->ht_ctxt.reset();
    proxyccb->hal_handle_ht_ctxt.reset();

    return proxyccb;
}

// allocate and initialize a PROXYCCB instance
static inline proxyccb_t *
proxyccb_alloc_init (void)
{
    return proxyccb_init(proxyccb_alloc());
}

static inline hal_ret_t
proxyccb_free (proxyccb_t *proxyccb)
{
    HAL_SPINLOCK_DESTROY(&proxyccb->slock);
    g_hal_state->proxyccb_slab()->free(proxyccb);
    return HAL_RET_OK;
}

static inline proxyccb_t *
find_proxyccb_by_id (proxyccb_id_t proxyccb_id)
{
    return (proxyccb_t *)g_hal_state->proxyccb_id_ht()->lookup(&proxyccb_id);
}

static inline proxyccb_t *
find_proxyccb_by_handle (hal_handle_t handle)
{
    return (proxyccb_t *)g_hal_state->proxyccb_hal_handle_ht()->lookup(&handle);
}

extern void *proxyccb_get_key_func(void *entry);
extern uint32_t proxyccb_compute_hash_func(void *key, uint32_t ht_size);
extern bool proxyccb_compare_key_func(void *key1, void *key2);

extern void *proxyccb_get_handle_key_func(void *entry);
extern uint32_t proxyccb_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool proxyccb_compare_handle_key_func(void *key1, void *key2);

hal_ret_t proxyccb_create(proxyccb::ProxycCbSpec& spec,
                        proxyccb::ProxycCbResponse *rsp);

hal_ret_t proxyccb_update(proxyccb::ProxycCbSpec& spec,
                        proxyccb::ProxycCbResponse *rsp);

hal_ret_t proxyccb_delete(proxyccb::ProxycCbDeleteRequest& req,
                        proxyccb::ProxycCbDeleteResponseMsg *rsp);

hal_ret_t proxyccb_get(proxyccb::ProxycCbGetRequest& req,
                     proxyccb::ProxycCbGetResponse *rsp);
}    // namespace hal

#endif    // __PROXYCCB_HPP__

