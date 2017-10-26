#ifndef __RAWRCB_HPP__
#define __RAWRCB_HPP__

#include "nic/include/base.h"
#include "nic/include/encap.hpp"
#include "nic/include/list.hpp"
#include "nic/utils/ht/ht.hpp"
#include "nic/gen/proto/hal/rawrcb.pb.h"
#include "nic/include/pd.hpp"
#include "nic/include/hal_state.hpp"

using hal::utils::ht_ctxt_t;
using hal::utils::dllist_ctxt_t;

using rawrcb::RawrCbSpec;
using rawrcb::RawrCbStatus;
using rawrcb::RawrCbResponse;
using rawrcb::RawrCbKeyHandle;
using rawrcb::RawrCbRequestMsg;
using rawrcb::RawrCbResponseMsg;
using rawrcb::RawrCbDeleteRequestMsg;
using rawrcb::RawrCbDeleteResponseMsg;
using rawrcb::RawrCbGetRequest;
using rawrcb::RawrCbGetRequestMsg;
using rawrcb::RawrCbGetResponse;
using rawrcb::RawrCbGetResponseMsg;

namespace hal {


typedef uint32_t          rawrcb_id_t;

typedef struct rawrcb_s {
    hal_spinlock_t        slock;                   // lock to protect this structure
    rawrcb_id_t           cb_id;

    /*
     * Note that ordering of fields below does not matter;
     * data will get written to HBM according to P4+ table entry
     * ordering defined in rawr_rxdma_p4plus_ingress.h
     * see hal/pd/iris/rawrcb_pd.cc)
     */
    uint32_t              chain_rxq_base;           // next service chain RxQ base
    uint32_t              chain_rxq_ring_indices_addr;
    uint8_t               chain_rxq_ring_size_shift;
    uint8_t               chain_rxq_entry_size_shift;
    uint8_t               chain_rxq_ring_index_select;

    uint32_t              chain_txq_base;           // next service chain TxQ base, if any
    uint32_t              chain_txq_ring_indices_addr;
    uint32_t              chain_txq_qid;
    uint16_t              chain_txq_lif;
    uint8_t               chain_txq_qtype;
    uint8_t               chain_txq_ring_size_shift;
    uint8_t               chain_txq_entry_size_shift;
    uint8_t               chain_txq_ring_index_select;
    uint8_t               chain_txq_doorbell_no_sched;

    uint8_t               desc_valid_bit_upd;       // descriptor valid bit update
    uint8_t               desc_valid_bit_req;       // descriptor valid bit required
    uint8_t               redir_pipeline_lpbk_enable; // redir pipeline loopback indicator

    hal_handle_t          hal_handle;               // HAL allocated handle

    // PD state
    void                  *pd;                      // all PD specific state

    ht_ctxt_t             ht_ctxt;                  // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;       // hal handle based hash table ctxt
} __PACK__ rawrcb_t;

// max. number of RAWR CBs supported  (TODO: we can take this from cfg file)
#define HAL_MAX_RAWRCB                           2048

// allocate a RAWRCB instance
static inline rawrcb_t *
rawrcb_alloc (void)
{
    rawrcb_t    *rawrcb;

    rawrcb = (rawrcb_t *)g_hal_state->rawrcb_slab()->alloc();
    if (rawrcb == NULL) {
        return NULL;
    }
    return rawrcb;
}

// initialize a RAWRCB instance
static inline rawrcb_t *
rawrcb_init (rawrcb_t *rawrcb)
{
    if (!rawrcb) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&rawrcb->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    rawrcb->pd = NULL;
    rawrcb->desc_valid_bit_req = TRUE;

    // initialize meta information
    rawrcb->ht_ctxt.reset();
    rawrcb->hal_handle_ht_ctxt.reset();

    return rawrcb;
}

// allocate and initialize a RAWRCB instance
static inline rawrcb_t *
rawrcb_alloc_init (void)
{
    return rawrcb_init(rawrcb_alloc());
}

static inline hal_ret_t
rawrcb_free (rawrcb_t *rawrcb)
{
    HAL_SPINLOCK_DESTROY(&rawrcb->slock);
    g_hal_state->rawrcb_slab()->free(rawrcb);
    return HAL_RET_OK;
}

static inline rawrcb_t *
find_rawrcb_by_id (rawrcb_id_t rawrcb_id)
{
    return (rawrcb_t *)g_hal_state->rawrcb_id_ht()->lookup(&rawrcb_id);
}

static inline rawrcb_t *
find_rawrcb_by_handle (hal_handle_t handle)
{
    return (rawrcb_t *)g_hal_state->rawrcb_hal_handle_ht()->lookup(&handle);
}

extern void *rawrcb_get_key_func(void *entry);
extern uint32_t rawrcb_compute_hash_func(void *key, uint32_t ht_size);
extern bool rawrcb_compare_key_func(void *key1, void *key2);

extern void *rawrcb_get_handle_key_func(void *entry);
extern uint32_t rawrcb_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool rawrcb_compare_handle_key_func(void *key1, void *key2);

hal_ret_t rawrcb_create(rawrcb::RawrCbSpec& spec,
                        rawrcb::RawrCbResponse *rsp);

hal_ret_t rawrcb_update(rawrcb::RawrCbSpec& spec,
                        rawrcb::RawrCbResponse *rsp);

hal_ret_t rawrcb_delete(rawrcb::RawrCbDeleteRequest& req,
                        rawrcb::RawrCbDeleteResponseMsg *rsp);

hal_ret_t rawrcb_get(rawrcb::RawrCbGetRequest& req,
                     rawrcb::RawrCbGetResponse *rsp);
}    // namespace hal

#endif    // __RAWRCB_HPP__

