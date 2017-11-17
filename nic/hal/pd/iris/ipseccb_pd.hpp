#ifndef __HAL_PD_IPSECCB_HPP__
#define __HAL_PD_IPSECCB_HPP__

#include "nic/include/base.h"
#include "nic/utils/ht/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"

using hal::utils::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_IPSECCBS                        4

#define P4PD_IPSECCB_STAGE_ENTRY_OFFSET            64
#define P4PD_HBM_IPSEC_CB_ENTRY_SIZE               128

#define IPSEC_CB_RING_ENTRY_SIZE                   8
#define IPSEC_BARCO_RING_ENTRY_SIZE              128 

#define UDP_PORT_NAT_T 4500

typedef enum ipseccb_hwid_order_ {
    P4PD_HWID_IPSEC_RX_STAGE0 = 0,
    P4PD_HWID_IPSEC_IP_HDR = 1,
} ipseccb_hwid_order_t;

typedef enum ipsec_decrypt_hwid_order_ {
    P4PD_HWID_IPSEC_PART2 = 1,
} ipsec_decrypt_hwid_order_t;

typedef uint64_t    ipseccb_hw_id_t;

// ipseccb pd state

struct pd_ipseccb_encrypt_s {
    ipseccb_t           *ipseccb;              // PI IPSEC CB

    // operational state of ipseccb pd
    ipseccb_hw_id_t      hw_id;               // hw id for this ipseccb

    // meta data maintained for IPSEC CB pd
    ht_ctxt_t          hw_ht_ctxt;           // h/w id based hash table ctxt
} __PACK__;

struct pd_ipseccb_decrypt_s {
    ipseccb_t           *ipseccb;              // PI IPSEC CB

    // operational state of ipseccb pd
    ipseccb_hw_id_t      hw_id;               // hw id for this ipseccb

    // meta data maintained for IPSEC CB pd
    ht_ctxt_t          hw_ht_ctxt;           // h/w id based hash table ctxt
} __PACK__;

typedef struct pd_ipseccb_eth_ip4_hdr_s {
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
} __PACK__ pd_ipseccb_eth_ip4_hdr_t;

typedef struct pd_ipseccb_eth_ip6_hdr_s {
    mac_addr_t dmac;
    mac_addr_t smac;
    uint16_t   ethertype; 
    uint32_t   ver_tc_flowlabel;
    uint16_t   payload_length;
    uint8_t    next_hdr;
    uint8_t    hop_limit;
    uint8_t    src[16];
    uint8_t    dst[16];
} __PACK__ pd_ipseccb_eth_ip6_hdr_t;

typedef struct pd_ipseccb_udp_nat_t_hdr_s {
    uint16_t sport;
    uint16_t dport;
    uint16_t length;
    uint16_t csum;
} __PACK__ pd_ipseccb_udp_nat_t_hdr_t;

// allocate a ipseccb pd instance
static inline pd_ipseccb_encrypt_t *
ipseccb_pd_alloc (void)
{
    pd_ipseccb_encrypt_t    *ipseccb_pd;

    ipseccb_pd = (pd_ipseccb_encrypt_t *)g_hal_state_pd->ipseccb_slab()->alloc();
    if (ipseccb_pd == NULL) {
        return NULL;
    }

    return ipseccb_pd;
}

// allocate a ipseccb pd instance
static inline pd_ipseccb_decrypt_t *
ipseccb_pd_decrypt_alloc (void)
{
    pd_ipseccb_decrypt_t    *ipseccb_pd;

    ipseccb_pd = (pd_ipseccb_decrypt_t *)g_hal_state_pd->ipseccb_decrypt_slab()->alloc();
    if (ipseccb_pd == NULL) {
        return NULL;
    }

    return ipseccb_pd;
}

// initialize a ipseccb pd instance
static inline pd_ipseccb_encrypt_t *
ipseccb_pd_init (pd_ipseccb_encrypt_t *ipseccb_pd)
{
    if (!ipseccb_pd) {
        return NULL;
    }
    ipseccb_pd->ipseccb = NULL;

    // initialize meta information
    ipseccb_pd->hw_ht_ctxt.reset();

    return ipseccb_pd;
}

// initialize a ipseccb pd instance
static inline pd_ipseccb_decrypt_t *
ipseccb_pd_decrypt_init (pd_ipseccb_decrypt_t *ipseccb_pd)
{
    if (!ipseccb_pd) {
        return NULL;
    }
    ipseccb_pd->ipseccb = NULL;

    // initialize meta information
    ipseccb_pd->hw_ht_ctxt.reset();

    return ipseccb_pd;
}

// allocate and initialize a ipseccb pd instance
static inline pd_ipseccb_encrypt_t *
ipseccb_pd_alloc_init (void)
{
    return ipseccb_pd_init(ipseccb_pd_alloc());
}

// allocate and initialize a ipseccb pd instance
static inline pd_ipseccb_decrypt_t *
ipseccb_pd_decrypt_alloc_init (void)
{
    return ipseccb_pd_decrypt_init(ipseccb_pd_decrypt_alloc());
}

// free ipseccb pd instance
static inline hal_ret_t
ipseccb_pd_free (pd_ipseccb_encrypt_t *ipseccb_pd)
{
    g_hal_state_pd->ipseccb_slab()->free(ipseccb_pd);
    return HAL_RET_OK;
}

static inline hal_ret_t
ipseccb_pd_decrypt_free (pd_ipseccb_decrypt_t *ipseccb_pd)
{
    g_hal_state_pd->ipseccb_decrypt_slab()->free(ipseccb_pd);
    return HAL_RET_OK;
}

// insert ipseccb pd state in all meta data structures
static inline hal_ret_t
add_ipseccb_pd_to_db (pd_ipseccb_encrypt_t *ipseccb_pd)
{
    g_hal_state_pd->ipseccb_hwid_ht()->insert(ipseccb_pd, &ipseccb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// insert ipseccb pd state in all meta data structures
static inline hal_ret_t
add_ipseccb_pd_decrypt_to_db (pd_ipseccb_decrypt_t *ipseccb_pd)
{
    g_hal_state_pd->ipseccb_hwid_ht()->insert(ipseccb_pd, &ipseccb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

static inline hal_ret_t
del_ipseccb_pd_from_db(pd_ipseccb_encrypt_t *ipseccb_pd)
{
    g_hal_state_pd->ipseccb_hwid_ht()->remove(&ipseccb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

static inline hal_ret_t
del_ipseccb_pd_decrypt_from_db(pd_ipseccb_decrypt_t *ipseccb_pd)
{
    g_hal_state_pd->ipseccb_hwid_ht()->remove(&ipseccb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a ipseccb pd instance given its hw id
static inline pd_ipseccb_encrypt_t *
find_ipseccb_by_hwid (ipseccb_hw_id_t hwid)
{
    return (pd_ipseccb_encrypt_t *)g_hal_state_pd->ipseccb_hwid_ht()->lookup(&hwid);
}

// find a ipseccb pd instance given its hw id
static inline pd_ipseccb_decrypt_t *
find_ipseccb_decrypt_by_hwid (ipseccb_hw_id_t hwid)
{
    return (pd_ipseccb_decrypt_t *)g_hal_state_pd->ipseccb_hwid_ht()->lookup(&hwid);
}

extern void *ipseccb_pd_get_hw_key_func(void *entry);
extern uint32_t ipseccb_pd_compute_hw_hash_func(void *key, uint32_t ht_size);
extern bool ipseccb_pd_compare_hw_key_func(void *key1, void *key2);

extern void *ipseccb_pd_decrypt_get_hw_key_func(void *entry);
extern uint32_t ipseccb_pd_decrypt_compute_hw_hash_func(void *key, uint32_t ht_size);
extern bool ipseccb_pd_decrypt_compare_hw_key_func(void *key1, void *key2);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_IPSECCB_HPP__

