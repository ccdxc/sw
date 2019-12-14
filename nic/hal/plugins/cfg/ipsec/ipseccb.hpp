//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __IPSECCB_HPP__
#define __IPSECCB_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/encap.hpp"
#include "lib/list/list.hpp"
#include "lib/ht/ht.hpp"
#include "gen/proto/ipsec.pb.h"
#include "nic/include/pd.hpp"
#include "nic/hal/iris/include/hal_state.hpp"

using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;

using ipsec::IpsecCbSpec;
using ipsec::IpsecCbStatus;
using ipsec::IpsecCbResponse;
using ipsec::IpsecCbKeyHandle;
using ipsec::IpsecCbRequestMsg;
using ipsec::IpsecCbResponseMsg;
using ipsec::IpsecCbDeleteRequestMsg;
using ipsec::IpsecCbDeleteResponseMsg;
using ipsec::IpsecCbGetRequest;
using ipsec::IpsecCbGetRequestMsg;
using ipsec::IpsecCbGetResponse;
using ipsec::IpsecCbGetResponseMsg;

namespace hal {

typedef struct ipseccb_s {
    sdk_spinlock_t        slock;                   // lock to protect this structure
    ipseccb_id_t          cb_id;                   // CB id
    uint16_t              pi;
    uint16_t              ci;
    uint32_t              tunnel_sip4;
    uint32_t              tunnel_dip4;
    uint8_t               iv_size;
    uint8_t               icv_size;
    uint8_t               block_size;
    uint32_t              key_index;
    uint32_t              new_key_index;
    uint32_t              barco_enc_cmd;
    uint64_t              iv;
    uint32_t              iv_salt;
    uint32_t              esn_hi;
    uint32_t              esn_lo;
    uint32_t              spi;
    uint32_t              new_spi;
    uint32_t              expected_seq_no;
    uint64_t              seq_no_bmp;
    hal_handle_t          hal_handle;              // HAL allocated handle
    mac_addr_t            smac;
    mac_addr_t            dmac;
    ip_addr_t             sip6;
    ip_addr_t             dip6;
    uint8_t               is_v6;
    uint8_t               is_nat_t;
    uint8_t               is_random;
    uint8_t               extra_pad;
    uint8_t               flags;
    uint16_t              vrf_vlan;
    uint32_t              last_replay_seq_no;

    //Stats
    uint64_t              h2n_rx_pkts;
    uint64_t              h2n_rx_bytes;
    uint64_t              h2n_rx_drops;
    uint64_t              h2n_tx_pkts;
    uint64_t              h2n_tx_bytes;
    uint64_t              h2n_tx_drops;

    uint64_t              n2h_rx_pkts;
    uint64_t              n2h_rx_bytes;
    uint64_t              n2h_rx_drops;
    uint64_t              n2h_tx_pkts;
    uint64_t              n2h_tx_bytes;
    uint64_t              n2h_tx_drops;

    // PD state
    void                  *pd;                     // all PD specific state
    void                  *pd_decrypt;                     // all PD specific state

    ht_ctxt_t             ht_ctxt;                 // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;      // hal handle based hash table ctxt
} __PACK__ ipseccb_t;

// max. number of CBs supported  (TODO: we can take this from cfg file)
#define HAL_MAX_IPSECCB                           4

// allocate a ipseccbment instance
static inline ipseccb_t *
ipseccb_alloc (void)
{
    ipseccb_t    *ipseccb;

    ipseccb = (ipseccb_t *)g_hal_state->ipseccb_slab()->alloc();
    if (ipseccb == NULL) {
        return NULL;
    }
    return ipseccb;
}

// initialize a ipseccbment instance
static inline ipseccb_t *
ipseccb_init (ipseccb_t *ipseccb)
{
    if (!ipseccb) {
        return NULL;
    }
    SDK_SPINLOCK_INIT(&ipseccb->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    ipseccb->pd = NULL;

    // initialize meta information
    ipseccb->ht_ctxt.reset();
    ipseccb->hal_handle_ht_ctxt.reset();

    return ipseccb;
}

// allocate and initialize a IPSECCB instance
static inline ipseccb_t *
ipseccb_alloc_init (void)
{
    return ipseccb_init(ipseccb_alloc());
}

static inline hal_ret_t
ipseccb_free (ipseccb_t *ipseccb)
{
    SDK_SPINLOCK_DESTROY(&ipseccb->slock);
    hal::delay_delete_to_slab(HAL_SLAB_IPSECCB, ipseccb);
    return HAL_RET_OK;
}

static inline ipseccb_t *
find_ipseccb_by_id (ipseccb_id_t ipseccb_id)
{
    return (ipseccb_t *)g_hal_state->ipseccb_id_ht()->lookup(&ipseccb_id);
}

extern void *ipseccb_get_key_func(void *entry);
extern uint32_t ipseccb_key_size(void);

extern void *ipseccb_get_handle_key_func(void *entry);
extern uint32_t ipseccb_handle_key_size(void);

}    // namespace hal

#endif    // __IPSECCB_HPP__

