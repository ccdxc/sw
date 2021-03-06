//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __PROXYCCB_HPP__
#define __PROXYCCB_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/encap.hpp"
#include "lib/list/list.hpp"
#include "lib/ht/ht.hpp"
#include "gen/proto/internal.pb.h"
#include "nic/include/pd.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/include/app_redir_shared.h"

using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;

using internal::ProxycCbSpec;
using internal::ProxycCbStatus;
using internal::ProxycCbResponse;
using internal::ProxycCbKeyHandle;
using internal::ProxycCbRequestMsg;
using internal::ProxycCbResponseMsg;
using internal::ProxycCbDeleteRequestMsg;
using internal::ProxycCbDeleteResponseMsg;
using internal::ProxycCbGetRequest;
using internal::ProxycCbGetRequestMsg;
using internal::ProxycCbGetResponse;
using internal::ProxycCbGetResponseMsg;

namespace hal {

typedef struct proxyccb_s {
    sdk_spinlock_t        slock;                   // lock to protect this structure
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
    uint8_t               redir_span;

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
} proxyccb_t;

#define HAL_MAX_PROXYCCB_HT_SIZE          1024      // hash table size

/*
 * Number of PI/CI pairs defined in proxyccb_t above;
 * find a change the corresponding #define as needed.
 */
#define HAL_NUM_PROXYCCB_RINGS_MAX        APP_REDIR_PROXYC_RINGS_MAX

extern void *proxyccb_get_key_func(void *entry);
extern uint32_t proxyccb_key_size(void);

extern void *proxyccb_get_handle_key_func(void *entry);
extern uint32_t proxyccb_handle_key_size(void);
extern types::AppRedirType proxyccb_tcpcb_l7_proxy_type_eval(uint32_t flow_id);
extern types::AppRedirType proxyccb_tlscb_l7_proxy_type_eval(uint32_t flow_id);


}    // namespace hal

#endif    // __PROXYCCB_HPP__

