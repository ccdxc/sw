#ifndef __RAWCCB_HPP__
#define __RAWCCB_HPP__

#include "nic/include/base.h"
#include "nic/include/encap.hpp"
#include "nic/include/list.hpp"
#include "sdk/ht.hpp"
#include "nic/gen/proto/hal/rawccb.pb.h"
#include "nic/include/pd.hpp"
#include "nic/include/hal_state.hpp"

using sdk::lib::ht_ctxt_t;
using hal::utils::dllist_ctxt_t;

using rawccb::RawcCbSpec;
using rawccb::RawcCbStatus;
using rawccb::RawcCbResponse;
using rawccb::RawcCbKeyHandle;
using rawccb::RawcCbRequestMsg;
using rawccb::RawcCbResponseMsg;
using rawccb::RawcCbDeleteRequestMsg;
using rawccb::RawcCbDeleteResponseMsg;
using rawccb::RawcCbGetRequest;
using rawccb::RawcCbGetRequestMsg;
using rawccb::RawcCbGetResponse;
using rawccb::RawcCbGetResponseMsg;

namespace hal {

typedef struct rawccb_s {
    hal_spinlock_t        slock;                   // lock to protect this structure
    rawccb_id_t           cb_id;

    /*
     * Note that ordering of fields below does not matter;
     * data will get written to HBM according to P4+ table entry's
     * ordering defined in rawc_txdma_p4plus_ingress.h
     * see hal/pd/iris/rawccb_pd.cc)
     */
    uint16_t              pi;                       // if more PI/CI's are added, 
    uint16_t              ci;                       // be sure to adjust HAL_RAWCCB_NUM_RINGS_MAX below
    uint16_t              rawccb_flags;
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
} __PACK__ rawccb_t;

#define HAL_MAX_RAWCCB_HT_SIZE          1024        // hash table size

/*
 * Number of PI/CI pairs defined in rawccb_t above
 */
#define HAL_NUM_RAWCCB_RINGS_MAX        1

// allocate a RAWCCB instance
static inline rawccb_t *
rawccb_alloc (void)
{
    rawccb_t    *rawccb;

    rawccb = (rawccb_t *)g_hal_state->rawccb_slab()->alloc();
    if (rawccb == NULL) {
        return NULL;
    }
    return rawccb;
}

// initialize a RAWCCB instance
static inline rawccb_t *
rawccb_init (rawccb_t *rawccb)
{
    if (!rawccb) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&rawccb->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    rawccb->pd = NULL;

    // initialize meta information
    rawccb->ht_ctxt.reset();
    rawccb->hal_handle_ht_ctxt.reset();

    return rawccb;
}

// allocate and initialize a RAWCCB instance
static inline rawccb_t *
rawccb_alloc_init (void)
{
    return rawccb_init(rawccb_alloc());
}

static inline hal_ret_t
rawccb_free (rawccb_t *rawccb)
{
    HAL_SPINLOCK_DESTROY(&rawccb->slock);
    g_hal_state->rawccb_slab()->free(rawccb);
    return HAL_RET_OK;
}

static inline rawccb_t *
find_rawccb_by_id (rawccb_id_t rawccb_id)
{
    return (rawccb_t *)g_hal_state->rawccb_id_ht()->lookup(&rawccb_id);
}

static inline rawccb_t *
find_rawccb_by_handle (hal_handle_t handle)
{
    return (rawccb_t *)g_hal_state->rawccb_hal_handle_ht()->lookup(&handle);
}

extern void *rawccb_get_key_func(void *entry);
extern uint32_t rawccb_compute_hash_func(void *key, uint32_t ht_size);
extern bool rawccb_compare_key_func(void *key1, void *key2);

extern void *rawccb_get_handle_key_func(void *entry);
extern uint32_t rawccb_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool rawccb_compare_handle_key_func(void *key1, void *key2);

hal_ret_t rawccb_create(rawccb::RawcCbSpec& spec,
                        rawccb::RawcCbResponse *rsp);

hal_ret_t rawccb_update(rawccb::RawcCbSpec& spec,
                        rawccb::RawcCbResponse *rsp);

hal_ret_t rawccb_delete(rawccb::RawcCbDeleteRequest& req,
                        rawccb::RawcCbDeleteResponseMsg *rsp);

hal_ret_t rawccb_get(rawccb::RawcCbGetRequest& req,
                     rawccb::RawcCbGetResponse *rsp);
}    // namespace hal

#endif    // __RAWCCB_HPP__

