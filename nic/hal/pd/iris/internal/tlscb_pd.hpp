#ifndef __HAL_PD_TLSCB_HPP__
#define __HAL_PD_TLSCB_HPP__

#include "nic/include/base.h"
#include "sdk/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_TLSCBS                        2048

#define P4PD_TLSCB_STAGE_ENTRY_OFFSET            64

// NOTE : ** These offsets need to match the offsets in 
// nic/asm/proxy/tls/include/tls-table.h **
typedef enum tlscb_hwid_order_ {
    P4PD_HWID_TLS_TX_S0_T0_READ_TLS_STG0 = 0,
    P4PD_HWID_TLS_TX_S1_T0_READ_TLS_ST1_7 = 1,
    P4PD_HWID_TLS_TX_PRE_CRYPTO_STATS_U16 = 2,
    P4PD_HWID_TLS_TX_PRE_CRYPTO_STATS1_U64 = 3,
    P4PD_HWID_TLS_TX_PRE_CRYPTO_STATS2_U64 = 4,
    P4PD_HWID_TLS_TX_POST_CRYPTO_STATS_U16 = 5,
    P4PD_HWID_TLS_TX_POST_CRYPTO_STATS1_U64 = 6,
    P4PD_HWID_TLS_TX_POST_CRYPTO_STATS2_U64 = 7,
    P4PD_HWID_TLS_TX_CONFIG                 = 8,
    P4PD_HWID_TLS_TX_OPER_DATA_1_U64        = 9,
    P4PD_HWID_TLS_TX_OPER_DATA_2_U64        = 10,
    P4PD_HWID_TLS_TX_OPER_DATA_3_U64        = 11,
    P4PD_HWID_TLS_TX_OPER_DATA_4_U64        = 12,
    P4PD_HWID_TLS_TX_OPER_DATA_5_U64        = 13,
    P4PD_HWID_TLS_TX_OPER_DATA_6_U64        = 14,
    P4PD_HWID_TLS_TX_OPER_DATA_7_U64        = 15,
} tlscb_hwid_order_t;

typedef uint64_t    tlscb_hw_id_t;
// Atomic stats for TLS TX pre crypto in TLS CB
typedef struct __attribute__((__packed__)) __tls_tx_pre_crypto_stats1_t {
    uint64_t tnmdr_alloc;
    uint64_t enc_requests;
    uint64_t mac_requests;
    uint64_t stats3;
    uint64_t stats4;
    uint64_t stats5;
    uint64_t stats6;
    uint64_t stats7;
} tls_tx_pre_crypto_stats1_t;

// Atomic stats for TLS TX pre crypto in TLS CB
typedef struct __attribute__((__packed__)) __tls_tx_pre_crypto_stats2_t {
    uint64_t tnmpr_alloc;
    uint64_t dec_requests;
    uint64_t mac_requests;
    uint64_t stats3;
    uint64_t stats4;
    uint64_t stats5;
    uint64_t stats6;
    uint64_t stats7;
} tls_tx_pre_crypto_stats2_t;

// Atomic stats for TLS TX post crypto in TLS CB
typedef struct __attribute__((__packed__)) __tls_tx_post_crypto_stats1_t {
    uint64_t rnmdr_free;
    uint64_t enc_completions;
    uint64_t enc_failures;
    uint64_t mac_completions;
    uint64_t mac_failures;
    uint64_t stats5;
    uint64_t stats6;
    uint64_t stats7;
} tls_tx_post_crypto_stats1_t;

// Atomic stats for TLS TX post crypto in TLS CB
typedef struct __attribute__((__packed__)) __tls_tx_post_crypto_stats2_t {
    uint64_t rnmpr_free;
    uint64_t dec_completions;
    uint64_t dec_failures;
    uint64_t mac_completions;
    uint64_t mac_failures;
    uint64_t stats5;
    uint64_t stats6;
    uint64_t stats7;
} tls_tx_post_crypto_stats2_t;

// tlscb pd state
struct pd_tlscb_s {
    tlscb_t           *tlscb;              // PI TLS CB

    // operational state of tlscb pd
    tlscb_hw_id_t      hw_id;               // hw id for this tlscb

    // meta data maintained for TLS CB pd
    ht_ctxt_t          hw_ht_ctxt;           // h/w id based hash table ctxt
} __PACK__;

// allocate a tlscb pd instance
static inline pd_tlscb_t *
tlscb_pd_alloc (void)
{
    pd_tlscb_t    *tlscb_pd;

    tlscb_pd = (pd_tlscb_t *)g_hal_state_pd->tlscb_slab()->alloc();
    if (tlscb_pd == NULL) {
        return NULL;
    }

    return tlscb_pd;
}

// initialize a tlscb pd instance
static inline pd_tlscb_t *
tlscb_pd_init (pd_tlscb_t *tlscb_pd)
{
    if (!tlscb_pd) {
        return NULL;
    }
    tlscb_pd->tlscb = NULL;

    // initialize meta information
    tlscb_pd->hw_ht_ctxt.reset();

    return tlscb_pd;
}

// allocate and initialize a tlscb pd instance
static inline pd_tlscb_t *
tlscb_pd_alloc_init (void)
{
    return tlscb_pd_init(tlscb_pd_alloc());
}

// free tlscb pd instance
static inline hal_ret_t
tlscb_pd_free (pd_tlscb_t *tlscb_pd)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_TLSCB_PD, tlscb_pd);
    return HAL_RET_OK;
}

// insert tlscb pd state in all meta data structures
static inline hal_ret_t
add_tlscb_pd_to_db (pd_tlscb_t *tlscb_pd)
{
    g_hal_state_pd->tlscb_hwid_ht()->insert(tlscb_pd, &tlscb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

static inline hal_ret_t
del_tlscb_pd_from_db(pd_tlscb_t *tlscb_pd)
{
    g_hal_state_pd->tlscb_hwid_ht()->remove(&tlscb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a tlscb pd instance given its hw id
static inline pd_tlscb_t *
find_tlscb_by_hwid (tlscb_hw_id_t hwid)
{
    return (pd_tlscb_t *)g_hal_state_pd->tlscb_hwid_ht()->lookup(&hwid);
}

extern void *tlscb_pd_get_hw_key_func(void *entry);
extern uint32_t tlscb_pd_compute_hw_hash_func(void *key, uint32_t ht_size);
extern bool tlscb_pd_compare_hw_key_func(void *key1, void *key2);
extern hal_ret_t p4pd_get_tls_tx_s5_t0_post_crypto_stats_entry(pd_tlscb_t* tlscb_pd);

uint32_t pd_tlscb_serq_ci_offset_get(void);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_TLSCB_HPP__

