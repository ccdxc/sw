#ifndef __HAL_PD_TCPCB_HPP__
#define __HAL_PD_TCPCB_HPP__

#include "nic/include/base.hpp"
#include "lib/ht/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/tcp_common.h"
#include "nic/sdk/asic/cmn/asic_common.hpp"

using sdk::lib::ht_ctxt_t;
using namespace sdk::platform::capri;

namespace hal {
namespace pd {

#define HAL_MAX_HW_TCPCBS                        2048

typedef uint64_t    tcpcb_hw_id_t;

// tcpcb pd state
struct pd_tcpcb_s {
    tcpcb_t           *tcpcb;              // PI TCP CB

    // operational state of tcpcb pd
    tcpcb_hw_id_t      hw_id;               // hw id for this tcpcb
    tcpcb_hw_id_t      hw_id_qtype1;        // hw id for qtype 1

    // meta data maintained for TCP CB pd
    ht_ctxt_t          hw_ht_ctxt;           // h/w id based hash table ctxt
} __PACK__;

// allocate a tcpcb pd instance
static inline pd_tcpcb_t *
tcpcb_pd_alloc (void)
{
    pd_tcpcb_t    *tcpcb_pd;

    tcpcb_pd = (pd_tcpcb_t *)g_hal_state_pd->tcpcb_slab()->alloc();
    if (tcpcb_pd == NULL) {
        return NULL;
    }

    return tcpcb_pd;
}

// initialize a tcpcb pd instance
static inline pd_tcpcb_t *
tcpcb_pd_init (pd_tcpcb_t *tcpcb_pd)
{
    if (!tcpcb_pd) {
        return NULL;
    }
    tcpcb_pd->tcpcb = NULL;

    // initialize meta information
    tcpcb_pd->hw_ht_ctxt.reset();

    return tcpcb_pd;
}

// allocate and initialize a tcpcb pd instance
static inline pd_tcpcb_t *
tcpcb_pd_alloc_init (void)
{
    return tcpcb_pd_init(tcpcb_pd_alloc());
}

// free tcpcb pd instance
static inline hal_ret_t
tcpcb_pd_free (pd_tcpcb_t *tcpcb_pd)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_TCPCB_PD, tcpcb_pd);
    return HAL_RET_OK;
}

// insert tcpcb pd state in all meta data structures
static inline hal_ret_t
add_tcpcb_pd_to_db (pd_tcpcb_t *tcpcb_pd)
{
    g_hal_state_pd->tcpcb_hwid_ht()->insert(tcpcb_pd, &tcpcb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

static inline hal_ret_t
del_tcpcb_pd_from_db(pd_tcpcb_t *tcpcb_pd)
{
    g_hal_state_pd->tcpcb_hwid_ht()->remove(&tcpcb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a tcpcb pd instance given its hw id
static inline pd_tcpcb_t *
find_tcpcb_by_hwid (tcpcb_hw_id_t hwid)
{
    return (pd_tcpcb_t *)g_hal_state_pd->tcpcb_hwid_ht()->lookup(&hwid);
}

extern void *tcpcb_pd_get_hw_key_func(void *entry);
extern uint32_t tcpcb_pd_hw_key_size(void);
extern uint64_t tcpcb_pd_serq_prod_ci_addr_get(uint32_t qid);
extern hal_ret_t tcpcb_pd_serq_lif_qtype_qstate_ring_set (uint32_t tcp_qid,
                                                          uint32_t lif,
                                                          uint32_t qtype,
                                                          uint32_t qid,
                                                          uint32_t ring);

//Short term fix. Bypasses state
hal_ret_t
p4pd_update_sesq_ci_addr(uint32_t qid, uint64_t ci_addr);

static inline
uint64_t tcpcb_sesq_db_addr (uint32_t qid)
{
    return _ASIC_SETUP_DB_ADDR(DB_IDX_UPD_PIDX_SET, DB_SCHED_UPD_EVAL,
                                SERVICE_LIF_TCP_PROXY, 0 /*QTYPE*/);
}

static inline
uint64_t tcpcb_sesq_db_data (uint32_t qid)
{
    return _ASIC_SETUP_DB_DATA(qid, 0 /*ring_id*/, 0 /*pindex*/);
}


}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_TCPCB_HPP__

