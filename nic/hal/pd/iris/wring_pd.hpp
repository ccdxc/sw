#ifndef __HAL_PD_WRING_HPP__
#define __HAL_PD_WRING_HPP__

#include <base.h>
#include <ht.hpp>
#include <hal_state_pd.hpp>

using hal::utils::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_WRING                        2048
#define WRING_SLOT_SIZE                         sizeof(uint64_t)

typedef uint32_t    wring_hw_id_t;

// wring pd state
struct pd_wring_s {
    wring_t           *wring;              // PI TCP CB

    // operational state of wring pd
    wring_hw_id_t      hw_id;               // hw id for this wring

    // meta data maintained for TCP CB pd
    ht_ctxt_t          hw_ht_ctxt;           // h/w id based hash table ctxt
} __PACK__;

typedef struct pd_wring_meta_s {
    bool        is_global;
    char        hbm_reg_name[64];
    uint32_t    num_slots;
    char        obj_hbm_reg_name[64];
    uint32_t    obj_size;
} pd_wring_meta_t;

// allocate a wring pd instance
static inline pd_wring_t *
wring_pd_alloc (void)
{
    pd_wring_t    *wring_pd;

    wring_pd = (pd_wring_t *)g_hal_state_pd->wring_slab()->alloc();
    if (wring_pd == NULL) {
        return NULL;
    }

    return wring_pd;
}

// initialize a wring pd instance
static inline pd_wring_t *
wring_pd_init (pd_wring_t *wring_pd)
{
    if (!wring_pd) {
        return NULL;
    }
    wring_pd->wring = NULL;

    // initialize meta information
    wring_pd->hw_ht_ctxt.reset();

    return wring_pd;
}

// allocate and initialize a wring pd instance
static inline pd_wring_t *
wring_pd_alloc_init (void)
{
    return wring_pd_init(wring_pd_alloc());
}

// free wring pd instance
static inline hal_ret_t
wring_pd_free (pd_wring_t *wring_pd)
{
    g_hal_state_pd->wring_slab()->free(wring_pd);
    return HAL_RET_OK;
}

// insert wring pd state in all meta data structures
static inline hal_ret_t
add_wring_pd_to_db (pd_wring_t *wring_pd)
{
    g_hal_state_pd->wring_hwid_ht()->insert(wring_pd, &wring_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a wring pd instance given its hw id
static inline pd_wring_t *
find_wring_by_hwid (wring_hw_id_t hwid)
{
    return (pd_wring_t *)g_hal_state_pd->wring_hwid_ht()->lookup(&hwid);
}

hal_ret_t wring_pd_init_global_rings(void);

// Get the base address for the ring
hal_ret_t wring_pd_get_base_addr(types::WRingType type,
                                 uint32_t wring_id,
                                 wring_hw_id_t* wring_base);

extern void *wring_pd_get_hw_key_func(void *entry);
extern uint32_t wring_pd_compute_hw_hash_func(void *key, uint32_t ht_size);
extern bool wring_pd_compare_hw_key_func(void *key1, void *key2);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_WRING_HPP__

