//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __RAWRCB_HPP__
#define __RAWRCB_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/encap.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "lib/list/list.hpp"
#include "lib/ht/ht.hpp"
#include "gen/proto/internal.pb.h"
#include "nic/include/pd.hpp"
#include "nic/include/app_redir_shared.h"

using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;

using internal::RawrCbSpec;
using internal::RawrCbStatus;
using internal::RawrCbResponse;
using internal::RawrCbKeyHandle;
using internal::RawrCbRequestMsg;
using internal::RawrCbResponseMsg;
using internal::RawrCbDeleteRequestMsg;
using internal::RawrCbDeleteResponseMsg;
using internal::RawrCbGetRequest;
using internal::RawrCbGetRequestMsg;
using internal::RawrCbGetResponse;
using internal::RawrCbGetResponseMsg;

namespace hal {


typedef struct rawrcb_s {
    sdk_spinlock_t        slock;                   // lock to protect this structure
    rawrcb_id_t           cb_id;

    /*
     * Note that ordering of fields below does not matter;
     * data will get written to HBM according to P4+ table entry's
     * ordering defined in rawr_rxdma_p4plus_ingress.h
     * see hal/pd/iris/rawrcb_pd.cc)
     */
    uint16_t              rawrcb_flags;
    uint64_t              chain_rxq_base;           // next service chain RxQ base
    uint64_t              chain_rxq_ring_indices_addr;
    uint8_t               chain_rxq_ring_size_shift;
    uint8_t               chain_rxq_entry_size_shift;
    uint8_t               chain_rxq_ring_index_select;

    uint64_t              chain_txq_base;           // next service chain TxQ base, if any
    uint64_t              chain_txq_ring_indices_addr;
    uint32_t              chain_txq_qid;
    uint16_t              chain_txq_lif;
    uint8_t               chain_txq_qtype;
    uint8_t               chain_txq_ring_size_shift;
    uint8_t               chain_txq_entry_size_shift;
    uint8_t               chain_txq_ring_index_select;
    uint8_t               cpu_id;
    uint64_t              ascq_base;
    uint64_t              ascq_sem_inf_addr;

    /*
     * 64-bit statistic counters
     */
    uint64_t              redir_pkts;
    uint64_t              cb_not_ready_discards;
    uint64_t              qstate_cfg_discards;
    uint64_t              pkt_len_discards;
    uint64_t              rxq_full_discards;
    uint64_t              txq_full_discards;
    uint64_t              pkt_alloc_errors;
    uint64_t              pkt_free_errors;

    hal_handle_t          hal_handle;               // HAL allocated handle

    // PD state
    void                  *pd;                      // all PD specific state

    ht_ctxt_t             ht_ctxt;                  // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;       // hal handle based hash table ctxt
} rawrcb_t;

#define HAL_MAX_RAWRCB_HT_SIZE          1024        // hash table size

/*
 * Number of PI/CI pairs defined in rawrcb_t above;
 * find a change the corresponding #define as needed.
 */
#define HAL_NUM_RAWRCB_RINGS_MAX        APP_REDIR_RAWR_RINGS_MAX

extern void *rawrcb_get_key_func(void *entry);
extern uint32_t rawrcb_key_size(void);
extern rawrcb_t *find_rawrcb_by_id(rawrcb_id_t rawrcb_id);
extern rawrcb_t *find_rawrcb_by_handle(hal_handle_t handle);

extern void *rawrcb_get_handle_key_func(void *entry);
extern uint32_t rawrcb_handle_key_size(void *key, uint32_t ht_size);

hal_ret_t rawrcb_create(internal::RawrCbSpec& spec,
                        internal::RawrCbResponse *rsp);

hal_ret_t rawrcb_update(internal::RawrCbSpec& spec,
                        internal::RawrCbResponse *rsp);

hal_ret_t rawrcb_delete(internal::RawrCbDeleteRequest& req,
                        internal::RawrCbDeleteResponseMsg *rsp);

hal_ret_t rawrcb_get(internal::RawrCbGetRequest& req,
                     internal::RawrCbGetResponse *rsp);
}    // namespace hal

#endif    // __RAWRCB_HPP__

