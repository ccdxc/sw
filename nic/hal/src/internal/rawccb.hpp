//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __RAWCCB_HPP__
#define __RAWCCB_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/encap.hpp"
#include "lib/list/list.hpp"
#include "lib/ht/ht.hpp"
#include "gen/proto/internal.pb.h"
#include "nic/include/pd.hpp"
#include "nic/include/app_redir_shared.h"

using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;

using internal::RawcCbSpec;
using internal::RawcCbStatus;
using internal::RawcCbResponse;
using internal::RawcCbKeyHandle;
using internal::RawcCbRequestMsg;
using internal::RawcCbResponseMsg;
using internal::RawcCbDeleteRequestMsg;
using internal::RawcCbDeleteResponseMsg;
using internal::RawcCbGetRequest;
using internal::RawcCbGetRequestMsg;
using internal::RawcCbGetResponse;
using internal::RawcCbGetResponseMsg;

namespace hal {

typedef struct rawccb_s {
    sdk_spinlock_t        slock;                   // lock to protect this structure
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
    uint8_t               cpu_id;
    uint64_t              ascq_base;
    uint64_t              ascq_sem_inf_addr;

    /*
     * 64-bit statistic counters
     */
    uint64_t              chain_pkts;
    uint64_t              cb_not_ready_discards;
    uint64_t              qstate_cfg_discards;
    uint64_t              aol_error_discards;
    uint64_t              my_txq_empty_discards;
    uint64_t              txq_full_discards;
    uint64_t              pkt_free_errors;

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
extern uint32_t rawccb_key_size(void);

extern void *rawccb_get_handle_key_func(void *entry);
extern uint32_t rawccb_handle_key_size(void);

}    // namespace hal

#endif    // __RAWCCB_HPP__

