#ifndef __RAWCCB_HPP__
#define __RAWCCB_HPP__

#include "nic/include/base.h"
#include "nic/include/encap.hpp"
#include "sdk/list.hpp"
#include "sdk/ht.hpp"
#include "nic/gen/proto/hal/rawccb.pb.h"
#include "nic/include/pd.hpp"
#include "nic/include/app_redir_shared.h"

using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;

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

    /*
     * 64-bit statistic counters
     */
    uint64_t              stat_pkts_chain;
    uint64_t              stat_pkts_discard;

    /*
     * 32-bit saturating statistic counters
     */
    uint32_t              stat_cb_not_ready;
    uint32_t              stat_my_txq_empty;
    uint32_t              stat_aol_err;
    uint32_t              stat_txq_full;
    uint32_t              stat_desc_sem_free_full;
    uint32_t              stat_page_sem_free_full;

    hal_handle_t          hal_handle;               // HAL allocated handle

    // PD state
    void                  *pd;                      // all PD specific state

    ht_ctxt_t             ht_ctxt;                  // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;       // hal handle based hash table ctxt
} rawccb_t;

#define HAL_MAX_RAWCCB_HT_SIZE          1024        // hash table size

/*
 * Number of PI/CI pairs defined in rawccb_t above;
 * find a change the corresponding #define as needed.
 */
#define HAL_NUM_RAWCCB_RINGS_MAX        APP_REDIR_RAWC_RINGS_MAX

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
                        rawccb::RawcCbDeleteResponse *rsp);

hal_ret_t rawccb_get(rawccb::RawcCbGetRequest& req,
                     rawccb::RawcCbGetResponseMsg *rsp);
}    // namespace hal

#endif    // __RAWCCB_HPP__

