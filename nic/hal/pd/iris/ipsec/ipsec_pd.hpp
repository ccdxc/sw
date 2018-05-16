#ifndef __HAL_PD_IPSEC_HPP__
#define __HAL_PD_IPSEC_HPP__

#include "nic/include/base.h"
#include "sdk/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_IPSEC_SA                        2048 

#define P4PD_IPSECCB_STAGE_ENTRY_OFFSET            64
#define P4PD_HBM_IPSEC_CB_ENTRY_SIZE               128

#define IPSEC_CB_RING_ENTRY_SIZE                   8
#define IPSEC_BARCO_RING_ENTRY_SIZE              128 

#define UDP_PORT_NAT_T 4500

typedef enum ipsec_sa_hwid_order_ {
    P4PD_HWID_IPSEC_QSTATE1 = 0,
    P4PD_HWID_IPSEC_ETH_IP_HDR = 1,
} ipsec_sa_hwid_order_t;

typedef enum ipsec_sa_hwid_order2_ {
    P4PD_HWID_IPSEC_QSTATE2 = 1,
} ipsec_sa_hwid_order2_t;

typedef uint64_t    ipsec_sa_hw_id_t;

// ipsec_sa pd state

struct pd_ipsec_s {
    ipsec_sa_t           *ipsec_sa;              // PI IPSEC CB

    // operational state of ipsec_sa pd
    ipsec_sa_hw_id_t      hw_id;               // hw id for this ipsec_sa

    // meta data maintained for IPSEC CB pd
    ht_ctxt_t          hw_ht_ctxt;           // h/w id based hash table ctxt
} __PACK__;

typedef struct pd_ipsec_eth_ip4_hdr_s {
    mac_addr_t dmac;
    mac_addr_t smac;
    uint16_t   ethertype; 
    uint8_t    version_ihl;
    uint8_t    tos;
    uint16_t   tot_len;
    uint16_t   id;
    uint16_t   frag_off;
    uint8_t    ttl;
    uint8_t    protocol;
    uint16_t   check;
    uint32_t   saddr;
    uint32_t   daddr;
} __PACK__ pd_ipsec_eth_ip4_hdr_t;

typedef struct pd_ipsec_eth_ip6_hdr_s {
    mac_addr_t dmac;
    mac_addr_t smac;
    uint16_t   ethertype; 
    uint32_t   ver_tc_flowlabel;
    uint16_t   payload_length;
    uint8_t    next_hdr;
    uint8_t    hop_limit;
    uint8_t    src[16];
    uint8_t    dst[16];
} __PACK__ pd_ipsec_eth_ip6_hdr_t;

typedef struct pd_ipsec_udp_nat_t_hdr_s {
    uint16_t sport;
    uint16_t dport;
    uint16_t length;
    uint16_t csum;
} __PACK__ pd_ipsec_udp_nat_t_hdr_t;

typedef struct pd_ipsec_decrypt_part2_s {
    uint32_t spi;
    uint32_t new_spi;
    uint32_t last_replay_seq_no;
    uint32_t iv_salt;
} __PACK__ pd_ipsec_decrypt_part2_t;

typedef struct pd_ipsec_qstate_addr_part2_s {
    union {
       pd_ipsec_eth_ip4_hdr_t eth_ip4_hdr;
       pd_ipsec_eth_ip6_hdr_t eth_ip6_hdr;
    } u;
    pd_ipsec_udp_nat_t_hdr_t nat_t_hdr;
    uint8_t pad[2];
} __PACK__ pd_ipsec_qstate_addr_part2_t;

 
// allocate a ipsec_sa pd instance
static inline pd_ipsec_t *
ipsec_pd_encrypt_alloc (void)
{
    pd_ipsec_t    *ipsec_sa_pd;

    ipsec_sa_pd = (pd_ipsec_t *)g_hal_state_pd->ipsec_sa_slab()->alloc();
    if (ipsec_sa_pd == NULL) {
        return NULL;
    }

    return ipsec_sa_pd;
}

// allocate a ipsec_sa pd instance
static inline pd_ipsec_t *
ipsec_pd_decrypt_alloc (void)
{
    pd_ipsec_t    *ipsec_sa_pd;

    ipsec_sa_pd = (pd_ipsec_t *)g_hal_state_pd->ipsec_sa_slab()->alloc();
    if (ipsec_sa_pd == NULL) {
        return NULL;
    }

    return ipsec_sa_pd;
}

// initialize a ipsec_sa pd instance
static inline pd_ipsec_t *
ipsec_pd_encrypt_init (pd_ipsec_t *ipsec_sa_pd)
{
    if (!ipsec_sa_pd) {
        return NULL;
    }
    ipsec_sa_pd->ipsec_sa = NULL;

    // initialize meta information
    ipsec_sa_pd->hw_ht_ctxt.reset();

    return ipsec_sa_pd;
}

// initialize a ipsec_sa pd instance
static inline pd_ipsec_t *
ipsec_pd_decrypt_init (pd_ipsec_t *ipsec_sa_pd)
{
    if (!ipsec_sa_pd) {
        return NULL;
    }
    ipsec_sa_pd->ipsec_sa = NULL;

    // initialize meta information
    ipsec_sa_pd->hw_ht_ctxt.reset();

    return ipsec_sa_pd;
}

// allocate and initialize a ipsec_sa pd instance
static inline pd_ipsec_t *
ipsec_pd_alloc_init (void)
{
    return ipsec_pd_encrypt_init(ipsec_pd_encrypt_alloc());
}

// allocate and initialize a ipsec_sa pd instance
static inline pd_ipsec_t *
ipsec_pd_decrypt_alloc_init (void)
{
    return ipsec_pd_decrypt_init(ipsec_pd_decrypt_alloc());
}

// free ipsec_sa pd instance
static inline hal_ret_t
ipsec_pd_free (pd_ipsec_t *ipsec_sa_pd)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_IPSEC_SA_PD, ipsec_sa_pd);
    return HAL_RET_OK;
}

static inline hal_ret_t
ipsec_pd_decrypt_free (pd_ipsec_t *ipsec_sa_pd)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_IPSEC_SA_PD, ipsec_sa_pd);
    return HAL_RET_OK;
}

// insert ipsec_sa pd state in all meta data structures
static inline hal_ret_t
add_ipsec_pd_to_db (pd_ipsec_t *ipsec_sa_pd)
{
    g_hal_state_pd->ipsec_sa_hwid_ht()->insert(ipsec_sa_pd, &ipsec_sa_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// insert ipsec_sa pd state in all meta data structures
static inline hal_ret_t
add_ipsec_pd_decrypt_to_db (pd_ipsec_t *ipsec_sa_pd)
{
    g_hal_state_pd->ipsec_sa_hwid_ht()->insert(ipsec_sa_pd, &ipsec_sa_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

static inline hal_ret_t
del_ipsec_pd_from_db(pd_ipsec_t *ipsec_sa_pd)
{
    g_hal_state_pd->ipsec_sa_hwid_ht()->remove(&ipsec_sa_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

static inline hal_ret_t
del_ipsec_pd_decrypt_from_db(pd_ipsec_t *ipsec_sa_pd)
{
    g_hal_state_pd->ipsec_sa_hwid_ht()->remove(&ipsec_sa_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a ipsec_sa pd instance given its hw id
static inline pd_ipsec_t *
find_ipsec_by_hwid (ipsec_sa_hw_id_t hwid)
{
    return (pd_ipsec_t *)g_hal_state_pd->ipsec_sa_hwid_ht()->lookup(&hwid);
}

// find a ipsec_sa pd instance given its hw id
static inline pd_ipsec_t *
find_ipsec_decrypt_by_hwid (ipsec_sa_hw_id_t hwid)
{
    return (pd_ipsec_t *)g_hal_state_pd->ipsec_sa_hwid_ht()->lookup(&hwid);
}

extern void *ipsec_pd_get_hw_key_func(void *entry);
extern uint32_t ipsec_pd_compute_hw_hash_func(void *key, uint32_t ht_size);
extern bool ipsec_pd_compare_hw_key_func(void *key1, void *key2);
#if 0
extern void *ipsec_pd_decrypt_get_hw_key_func(void *entry);
extern uint32_t ipsec_pd_decrypt_compute_hw_hash_func(void *key, uint32_t ht_size);
extern bool ipsec_pd_decrypt_compare_hw_key_func(void *key1, void *key2);
#endif

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_IPSECCB_HPP__

