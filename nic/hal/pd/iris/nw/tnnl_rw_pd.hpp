#ifndef __HAL_TNNL_RW_PD_HPP__
#define __HAL_TNNL_RW_PD_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "lib/ht/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_TNNL_RW_TBL_ENTRIES                        1 * 1024    // size of tnnl rw table in P4. 

typedef struct pd_tnnl_rw_entry_info_s {
    bool         with_id;
    uint32_t     tnnl_rw_idx;
} pd_tnnl_rw_entry_info_t;

typedef struct pd_tnnl_rw_entry_key_s {
    mac_addr_t                  mac_sa;
    mac_addr_t                  mac_da;
    ip_addr_t                   ip_sa;
    ip_addr_t                   ip_da;
    uint8_t                     ip_type;
    uint8_t                     vlan_valid;
    uint16_t                    vlan_id;
    tunnel_rewrite_actions_en   tnnl_rw_act;
} __PACK__ pd_tnnl_rw_entry_key_t;

// rw table entry state
typedef struct pd_tnnl_rw_entry_s {
    pd_tnnl_rw_entry_key_t   tnnl_rw_key;
    uint32_t                 tnnl_rw_idx;          
    uint32_t                 ref_cnt;

    ht_ctxt_t                ht_ctxt; 
} __PACK__ pd_tnnl_rw_entry_t;

// allocate a rw entry instance
static inline pd_tnnl_rw_entry_t*
tnnl_rw_entry_pd_alloc (void)
{
    pd_tnnl_rw_entry_t       *rwe;

    rwe = (pd_tnnl_rw_entry_t *)g_hal_state_pd->tnnl_rw_entry_slab()->alloc();
    if (rwe == NULL) {
        return NULL;
    }

    return rwe;
}

// initialize a rwe instance
static inline pd_tnnl_rw_entry_t *
tnnl_rw_entry_pd_init (pd_tnnl_rw_entry_t *tnnl_rwe)
{
    if (!tnnl_rwe) {
        return NULL;
    }
    memset(&tnnl_rwe->tnnl_rw_key, 0, sizeof(pd_tnnl_rw_entry_key_t));
    tnnl_rwe->tnnl_rw_idx = 0;
    tnnl_rwe->ref_cnt = 0;

    // initialize meta information
    tnnl_rwe->ht_ctxt.reset();

    return tnnl_rwe;
}

// allocate and initialize a rw entry instance
static inline pd_tnnl_rw_entry_t *
tnnl_rw_entry_pd_alloc_init (void)
{
    return tnnl_rw_entry_pd_init(tnnl_rw_entry_pd_alloc());
}

// free rw entry instance
static inline hal_ret_t
tnnl_rw_entry_pd_free (pd_tnnl_rw_entry_t *tnnl_rwe)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_TUNNEL_RW_PD, tnnl_rwe);
    return HAL_RET_OK;
}

// insert rw entry state in all meta data structures
static inline hal_ret_t
add_tnnl_rw_entry_pd_to_db (pd_tnnl_rw_entry_t *tnnl_rwe)
{
    g_hal_state_pd->tnnl_rw_table_ht()->insert(tnnl_rwe, &tnnl_rwe->ht_ctxt);
    return HAL_RET_OK;
}

static inline hal_ret_t
del_tnnl_rw_entry_pd_from_db(pd_tnnl_rw_entry_t *tnnl_rwe)
{
    g_hal_state_pd->tnnl_rw_table_ht()->remove(&tnnl_rwe->tnnl_rw_key);
    return HAL_RET_OK;
}

// find a ipseccb pd instance given its hw id
static inline pd_tnnl_rw_entry_t *
find_tnnl_rw_entry_by_key (pd_tnnl_rw_entry_key_t *key) 
{
    return (pd_tnnl_rw_entry_t *)g_hal_state_pd->tnnl_rw_table_ht()->lookup(key);
}

// Walk tunnel rw hash table
static inline sdk_ret_t
walk_tnnl_rw_entry_ht (sdk::lib::ht::ht_walk_cb_t walk_cb, void *ctxt) 
{
    return (g_hal_state_pd->tnnl_rw_table_ht()->walk_safe(walk_cb, ctxt));
}

extern void *tnnl_rw_entry_pd_get_key_func(void *entry);
extern uint32_t tnnl_rw_entry_pd_key_size(void);

// Private APIs
hal_ret_t tnnl_rw_pd_pgm_tnnl_rw_tbl_vlan(pd_tnnl_rw_entry_t *tnnl_rwe);
hal_ret_t tnnl_rw_pd_pgm_tnnl_rw_tbl_vxlan(pd_tnnl_rw_entry_t *tnnl_rwe);
hal_ret_t tnnl_rw_pd_pgm_tnnl_rw_tbl_erspan(pd_tnnl_rw_entry_t *tnnl_rwe);
hal_ret_t tnnl_rw_pd_pgm_tnnl_rw_tbl_mpls_udp(pd_tnnl_rw_entry_t *tnnl_rwe);
hal_ret_t tnnl_rw_pd_depgm_tnnl_rw_tbl(pd_tnnl_rw_entry_t *tnnl_rwe);
hal_ret_t tnnl_rw_entry_find(pd_tnnl_rw_entry_key_t *tnnl_rw_key, pd_tnnl_rw_entry_t **tnnl_rwe);

// Public APIs
hal_ret_t tnnl_rw_entry_find_or_alloc(pd_tnnl_rw_entry_key_t *tnnl_rw_key, 
                                      uint32_t *tnnl_rw_idx, bool is_upgrade = false);
hal_ret_t tnnl_rw_entry_delete(pd_tnnl_rw_entry_key_t *tnnl_rw_key);
hal_ret_t tnnl_rw_entry_alloc(pd_tnnl_rw_entry_key_t *tnnl_rw_key, 
                              pd_tnnl_rw_entry_info_t *tnnl_rw_info, 
                              uint32_t *tnnl_rw_idx, bool is_upgrade = false);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_TNNL_RW_PD_HPP__

