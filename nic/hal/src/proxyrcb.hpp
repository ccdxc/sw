#ifndef __PROXYRCB_HPP__
#define __PROXYRCB_HPP__

#include "nic/include/base.h"
#include "nic/include/encap.hpp"
#include "nic/include/list.hpp"
#include "nic/utils/ht/ht.hpp"
#include "nic/gen/proto/hal/proxyrcb.pb.h"
#include "nic/include/pd.hpp"
#include "nic/include/hal_state.hpp"

using hal::utils::ht_ctxt_t;
using hal::utils::dllist_ctxt_t;

using proxyrcb::ProxyrCbSpec;
using proxyrcb::ProxyrCbStatus;
using proxyrcb::ProxyrCbResponse;
using proxyrcb::ProxyrCbKeyHandle;
using proxyrcb::ProxyrCbRequestMsg;
using proxyrcb::ProxyrCbResponseMsg;
using proxyrcb::ProxyrCbDeleteRequestMsg;
using proxyrcb::ProxyrCbDeleteResponseMsg;
using proxyrcb::ProxyrCbGetRequest;
using proxyrcb::ProxyrCbGetRequestMsg;
using proxyrcb::ProxyrCbGetResponse;
using proxyrcb::ProxyrCbGetResponseMsg;

namespace hal {


typedef uint32_t          proxyrcb_id_t;

typedef struct proxyrcb_s {
    hal_spinlock_t        slock;                   // lock to protect this structure
    proxyrcb_id_t         cb_id;

    /*
     * Note that ordering of fields below does not matter;
     * data will get written to HBM according to P4+ table entry's
     * ordering defined in proxyr_rxdma_p4plus_ingress.h
     * see hal/pd/iris/proxyrcb_pd.cc)
     */
    uint16_t              pi;                       // if more PI/CI's are added, 
    uint16_t              ci;                       // be sure to adjust HAL_PROXYRCB_NUM_RINGS_MAX below
    uint16_t              proxyrcb_flags;
    uint8_t               my_txq_ring_size_shift;
    uint8_t               my_txq_entry_size_shift;
    uint32_t              my_txq_base;

    uint32_t              chain_rxq_base;           // next service chain RxQ base
    uint32_t              chain_rxq_ring_indices_addr;
    uint8_t               chain_rxq_ring_size_shift;
    uint8_t               chain_rxq_entry_size_shift;
    uint8_t               chain_rxq_ring_index_select;

    /*
     * Flow keys for generating pen_proxy_redir_header_v1_t
     */
    ipvx_addr_t           ip_sa;
    ipvx_addr_t           ip_da;
    uint16_t              sport;
    uint16_t              dport;
    vrf_id_t              vrf;
    uint8_t               af;
    uint8_t               ip_proto;
    
    hal_handle_t          hal_handle;               // HAL allocated handle

    // PD state
    void                  *pd;                      // all PD specific state

    ht_ctxt_t             ht_ctxt;                  // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;       // hal handle based hash table ctxt
} __PACK__ proxyrcb_t;

#define HAL_MAX_PROXYRCB_HT_SIZE          1024      // hash table size

/*
 * Number of PI/CI pairs defined in proxyrcb_t above
 */
#define HAL_NUM_PROXYRCB_RINGS_MAX        1

// allocate a PROXYRCB instance
static inline proxyrcb_t *
proxyrcb_alloc (void)
{
    proxyrcb_t    *proxyrcb;

    proxyrcb = (proxyrcb_t *)g_hal_state->proxyrcb_slab()->alloc();
    if (proxyrcb == NULL) {
        return NULL;
    }
    return proxyrcb;
}

// initialize a PROXYRCB instance
static inline proxyrcb_t *
proxyrcb_init (proxyrcb_t *proxyrcb)
{
    if (!proxyrcb) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&proxyrcb->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    proxyrcb->pd = NULL;

    // initialize meta information
    proxyrcb->ht_ctxt.reset();
    proxyrcb->hal_handle_ht_ctxt.reset();

    return proxyrcb;
}

// allocate and initialize a PROXYRCB instance
static inline proxyrcb_t *
proxyrcb_alloc_init (void)
{
    return proxyrcb_init(proxyrcb_alloc());
}

static inline hal_ret_t
proxyrcb_free (proxyrcb_t *proxyrcb)
{
    HAL_SPINLOCK_DESTROY(&proxyrcb->slock);
    g_hal_state->proxyrcb_slab()->free(proxyrcb);
    return HAL_RET_OK;
}

static inline proxyrcb_t *
find_proxyrcb_by_id (proxyrcb_id_t proxyrcb_id)
{
    return (proxyrcb_t *)g_hal_state->proxyrcb_id_ht()->lookup(&proxyrcb_id);
}

static inline proxyrcb_t *
find_proxyrcb_by_handle (hal_handle_t handle)
{
    return (proxyrcb_t *)g_hal_state->proxyrcb_hal_handle_ht()->lookup(&handle);
}

extern void *proxyrcb_get_key_func(void *entry);
extern uint32_t proxyrcb_compute_hash_func(void *key, uint32_t ht_size);
extern bool proxyrcb_compare_key_func(void *key1, void *key2);

extern void *proxyrcb_get_handle_key_func(void *entry);
extern uint32_t proxyrcb_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool proxyrcb_compare_handle_key_func(void *key1, void *key2);

hal_ret_t proxyrcb_create(proxyrcb::ProxyrCbSpec& spec,
                          proxyrcb::ProxyrCbResponse *rsp);

hal_ret_t proxyrcb_update(proxyrcb::ProxyrCbSpec& spec,
                          proxyrcb::ProxyrCbResponse *rsp);

hal_ret_t proxyrcb_delete(proxyrcb::ProxyrCbDeleteRequest& req,
                          proxyrcb::ProxyrCbDeleteResponseMsg *rsp);

hal_ret_t proxyrcb_get(proxyrcb::ProxyrCbGetRequest& req,
                       proxyrcb::ProxyrCbGetResponse *rsp);
}    // namespace hal

#endif    // __PROXYRCB_HPP__

