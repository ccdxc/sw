#ifndef __HAL_PD_TCPCB_HPP__
#define __HAL_PD_TCPCB_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/tcp_common.h"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_TCPCBS                        2048

typedef uint64_t    tcp_proxy_cb_hw_id_t;

// tcpcb pd state
struct pd_tcp_proxy_cb_s {
    tcp_proxy_cb_t           *tcp_proxy_cb;              // PI TCP CB

    // operational state of tcpcb pd
    tcp_proxy_cb_hw_id_t      hw_id;               // hw id for this tcpcb

    // meta data maintained for TCP CB pd
    ht_ctxt_t          hw_ht_ctxt;           // h/w id based hash table ctxt
} __PACK__;

// allocate a tcpcb pd instance
static inline pd_tcp_proxy_cb_t *
tcp_proxy_cb_pd_alloc (void)
{
    pd_tcp_proxy_cb_t    *tcp_proxy_cb_pd;

    tcp_proxy_cb_pd = (pd_tcp_proxy_cb_t *)g_hal_state_pd->tcpcb_slab()->alloc();
    if (tcp_proxy_cb_pd == NULL) {
        return NULL;
    }

    return tcp_proxy_cb_pd;
}

// initialize a tcpcb pd instance
static inline pd_tcp_proxy_cb_t *
tcp_proxy_cb_pd_init (pd_tcp_proxy_cb_t *tcp_proxy_cb_pd)
{
    if (!tcp_proxy_cb_pd) {
        return NULL;
    }
    tcp_proxy_cb_pd->tcp_proxy_cb = NULL;

    // initialize meta information
    tcp_proxy_cb_pd->hw_ht_ctxt.reset();

    return tcp_proxy_cb_pd;
}

// allocate and initialize a tcpcb pd instance
static inline pd_tcp_proxy_cb_t *
tcp_proxy_cb_pd_alloc_init (void)
{
    return tcp_proxy_cb_pd_init(tcp_proxy_cb_pd_alloc());
}

// free tcpcb pd instance
static inline hal_ret_t
tcp_proxy_cb_pd_free (pd_tcp_proxy_cb_t *tcp_proxy_cb_pd)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_TCPCB_PD, tcp_proxy_cb_pd);
    return HAL_RET_OK;
}

// insert tcpcb pd state in all meta data structures
static inline hal_ret_t
add_tcp_proxy_cb_pd_to_db (pd_tcp_proxy_cb_t *tcp_proxy_cb_pd)
{
    g_hal_state_pd->tcpcb_hwid_ht()->insert(tcp_proxy_cb_pd, &tcp_proxy_cb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

static inline hal_ret_t
del_tcp_proxy_cb_pd_from_db(pd_tcp_proxy_cb_t *tcp_proxy_cb_pd)
{
    g_hal_state_pd->tcpcb_hwid_ht()->remove(&tcp_proxy_cb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a tcpcb pd instance given its hw id
static inline pd_tcp_proxy_cb_t *
find_tcp_proxy_cb_by_hwid (tcp_proxy_cb_hw_id_t hwid)
{
    return (pd_tcp_proxy_cb_t *)g_hal_state_pd->tcpcb_hwid_ht()->lookup(&hwid);
}

extern void *tcp_proxy_cb_pd_get_hw_key_func(void *entry);
extern uint32_t tcp_proxy_cb_pd_compute_hw_hash_func(void *key, uint32_t ht_size);
extern bool tcp_proxy_cb_pd_compare_hw_key_func(void *key1, void *key2);
extern uint64_t tcp_proxy_cb_pd_serq_prod_ci_addr_get(uint32_t qid);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_TCPCB_HPP__

