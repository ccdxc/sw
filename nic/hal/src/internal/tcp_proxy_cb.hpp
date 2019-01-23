//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __TCPCB_HPP__
#define __TCPCB_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/encap.hpp"
#include "lib/list/list.hpp"
#include "lib/ht/ht.hpp"
#include "gen/proto/internal.pb.h"
#include "nic/include/pd.hpp"
#include "nic/hal/iris/include/hal_state.hpp"

using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;


using internal::TcpCbSpec;
using internal::TcpCbStatus;
using internal::TcpCbResponse;
using internal::TcpCbKeyHandle;
using internal::TcpCbRequestMsg;
using internal::TcpCbResponseMsg;
using internal::TcpCbDeleteRequestMsg;
using internal::TcpCbDeleteResponseMsg;
using internal::TcpCbGetRequest;
using internal::TcpCbGetRequestMsg;
using internal::TcpCbGetResponse;
using internal::TcpCbGetResponseMsg;

#define INVALID_HEADER_TEMPLATE_LEN ((uint32_t)-1)

namespace hal {

typedef struct tcpcb_s {
    sdk_spinlock_t        slock;                   // lock to protect this structure
    tcpcb_id_t            cb_id;                   // TCP CB id
    uint32_t              rcv_nxt;
    uint32_t              snd_nxt;
    uint32_t              snd_una;
    uint32_t              rcv_tsval;
    uint32_t              ts_recent;
    uint64_t              rx_ts;
    uint64_t              serq_base;
    uint32_t              debug_dol;
    uint64_t              sesq_base;
    uint64_t              asesq_base;
    uint32_t              snd_wnd;
    uint32_t              snd_cwnd;
    uint32_t              rcv_mss;
    uint32_t              smss;
    uint16_t              source_port;
    uint16_t              dest_port;
    uint8_t               header_template[64];
    uint32_t              state;
    uint16_t              source_lif;
    uint32_t              debug_dol_tx;
    uint32_t              header_len;
    uint32_t              pending_ack_send;
    types::AppRedirType   l7_proxy_type;
    uint32_t              sesq_retx_ci;            // for testing, check in DOL
    uint32_t              asesq_retx_ci;           // for testing, check in DOL
    uint32_t              retx_snd_una;            // for testing, check in DOL
    uint32_t              sesq_tx_ci;              // for testing, check in DOL
    uint32_t              rto;
    uint32_t              snd_cwnd_cnt;
    uint32_t              serq_pi;
    uint32_t              serq_ci;
    uint32_t              pred_flags;
    uint32_t              rto_backoff;
    uint8_t               cpu_id;
    bool                  bypass_tls;
    uint32_t              snd_wscale;
    uint32_t              rcv_wscale;
    uint32_t              rcv_wnd;

    // operational state of TCP Proxy CB
    hal_handle_t          hal_handle;              // HAL allocated handle

    // rx stats
    uint32_t              debug_stage0_7_thread;
    uint64_t              bytes_rcvd;
    uint64_t              pkts_rcvd;
    uint64_t              pages_alloced;
    uint64_t              desc_alloced;
    uint64_t              debug_num_phv_to_mem;
    uint64_t              debug_num_pkt_to_mem;

    uint64_t              debug_atomic_delta;
    uint64_t              debug_atomic0_incr1247;
    uint64_t              debug_atomic1_incr247;
    uint64_t              debug_atomic2_incr47;
    uint64_t              debug_atomic3_incr47;
    uint64_t              debug_atomic4_incr7;
    uint64_t              debug_atomic5_incr7;
    uint64_t              debug_atomic6_incr7;

    uint64_t              bytes_acked;
    uint64_t              slow_path_cnt;
    uint64_t              serq_full_cnt;
    uint64_t              ooo_cnt;

    uint8_t               debug_dol_tblsetaddr;

    // tx stats
    uint64_t              bytes_sent;
    uint64_t              pkts_sent;
    uint64_t              debug_num_phv_to_pkt;
    uint64_t              debug_num_mem_to_pkt;

    uint32_t              sesq_pi;
    uint32_t              sesq_ci;
    uint32_t              send_ack_pi;
    uint32_t              send_ack_ci;
    uint32_t              del_ack_pi;
    uint32_t              del_ack_ci;
    uint32_t              fast_timer_pi;
    uint32_t              fast_timer_ci;
    uint32_t              asesq_pi;
    uint32_t              asesq_ci;
    uint32_t              pending_tx_pi;
    uint32_t              pending_tx_ci;
    uint32_t              fast_retrans_pi;
    uint32_t              fast_retrans_ci;
    uint32_t              clean_retx_pi;
    uint32_t              clean_retx_ci;
    uint32_t              packets_out;
    uint32_t              rto_pi;
    uint32_t              tx_ring_pi;
    uint32_t              partial_ack_cnt;
    bool                  delay_ack;
    uint32_t              ato;
    uint32_t              ato_deadline;
    uint32_t              rto_deadline;
    uint32_t              idle_deadline;
    uint32_t              snd_ssthresh;
    uint32_t              cc_algo;
    uint32_t              snd_recover;
    uint32_t              cc_flags;
    uint32_t              initial_window;

    // PD state
    void                  *pd;                     // all PD specific state

    ht_ctxt_t             ht_ctxt;                 // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;      // hal handle based hash table ctxt
    uint16_t              other_qid;
} __PACK__ tcpcb_t;

// max. number of TCP CBs supported  (TODO: we can take this from cfg file)
#define HAL_MAX_TCPCB                           2048

// allocate a tcpcbment instance
static inline tcpcb_t *
tcpcb_alloc (void)
{
    tcpcb_t    *tcpcb;

    tcpcb = (tcpcb_t *)g_hal_state->tcpcb_slab()->alloc();
    if (tcpcb == NULL) {
        return NULL;
    }
    return tcpcb;
}

// initialize a tcpcbment instance
static inline tcpcb_t *
tcpcb_init (tcpcb_t *tcpcb)
{
    if (!tcpcb) {
        return NULL;
    }
    SDK_SPINLOCK_INIT(&tcpcb->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    tcpcb->pd = NULL;

    // initialize meta information
    tcpcb->ht_ctxt.reset();
    tcpcb->hal_handle_ht_ctxt.reset();

    return tcpcb;
}

// allocate and initialize a TCPCB instance
static inline tcpcb_t *
tcpcb_alloc_init (void)
{
    return tcpcb_init(tcpcb_alloc());
}

static inline hal_ret_t
tcpcb_free (tcpcb_t *tcpcb)
{
    SDK_SPINLOCK_DESTROY(&tcpcb->slock);
    hal::delay_delete_to_slab(HAL_SLAB_TCPCB, tcpcb);
    return HAL_RET_OK;
}

static inline tcpcb_t *
find_tcpcb_by_id (tcpcb_id_t tcpcb_id)
{
    return (tcpcb_t *)g_hal_state->tcpcb_id_ht()->lookup(&tcpcb_id);
}

extern void *tcpcb_get_key_func(void *entry);
extern uint32_t tcpcb_compute_hash_func(void *key, uint32_t ht_size);
extern bool tcpcb_compare_key_func(void *key1, void *key2);

extern void *tcpcb_get_handle_key_func(void *entry);
extern uint32_t tcpcb_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool tcpcb_compare_handle_key_func(void *key1, void *key2);

}    // namespace hal

#endif    // __TCPCB_HPP__

