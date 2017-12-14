#ifndef __IPSECCB_HPP__
#define __IPSECCB_HPP__

#include "nic/include/base.h"
#include "nic/include/encap.hpp"
#include "nic/include/list.hpp"
#include "nic/sdk/include/ht.hpp"
#include "nic/gen/proto/hal/ipseccb.pb.h"
#include "nic/include/pd.hpp"
#include "nic/include/hal_state.hpp"

using sdk::lib::ht_ctxt_t;
using hal::utils::dllist_ctxt_t;

using ipseccb::IpsecCbSpec;
using ipseccb::IpsecCbStatus;
using ipseccb::IpsecCbResponse;
using ipseccb::IpsecCbKeyHandle;
using ipseccb::IpsecCbRequestMsg;
using ipseccb::IpsecCbResponseMsg;
using ipseccb::IpsecCbDeleteRequestMsg;
using ipseccb::IpsecCbDeleteResponseMsg;
using ipseccb::IpsecCbGetRequest;
using ipseccb::IpsecCbGetRequestMsg;
using ipseccb::IpsecCbGetResponse;
using ipseccb::IpsecCbGetResponseMsg;

namespace hal {

typedef struct ipseccb_s {
    hal_spinlock_t        slock;                   // lock to protect this structure
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
    uint8_t               flags;
    uint16_t              vrf_vlan;
    uint32_t              last_replay_seq_no;
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
    HAL_SPINLOCK_INIT(&ipseccb->slock, PTHREAD_PROCESS_PRIVATE);

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
    HAL_SPINLOCK_DESTROY(&ipseccb->slock);
    g_hal_state->ipseccb_slab()->free(ipseccb);
    return HAL_RET_OK;
}

static inline ipseccb_t *
find_ipseccb_by_id (ipseccb_id_t ipseccb_id)
{
    return (ipseccb_t *)g_hal_state->ipseccb_id_ht()->lookup(&ipseccb_id);
}

static inline ipseccb_t *
find_ipseccb_by_handle (hal_handle_t handle)
{
    return (ipseccb_t *)g_hal_state->ipseccb_hal_handle_ht()->lookup(&handle);
}

extern void *ipseccb_get_key_func(void *entry);
extern uint32_t ipseccb_compute_hash_func(void *key, uint32_t ht_size);
extern bool ipseccb_compare_key_func(void *key1, void *key2);

extern void *ipseccb_get_handle_key_func(void *entry);
extern uint32_t ipseccb_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool ipseccb_compare_handle_key_func(void *key1, void *key2);

hal_ret_t ipseccb_create(ipseccb::IpsecCbSpec& spec,
                       ipseccb::IpsecCbResponse *rsp);

hal_ret_t ipseccb_update(ipseccb::IpsecCbSpec& spec,
                       ipseccb::IpsecCbResponse *rsp);

hal_ret_t ipseccb_delete(ipseccb::IpsecCbDeleteRequest& req,
                       ipseccb::IpsecCbDeleteResponseMsg *rsp);

hal_ret_t ipseccb_get(ipseccb::IpsecCbGetRequest& req,
                    ipseccb::IpsecCbGetResponse *rsp);
}    // namespace hal

#endif    // __IPSECCB_HPP__

