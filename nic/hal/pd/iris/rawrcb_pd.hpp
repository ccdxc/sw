#ifndef __HAL_PD_RAWRCB_HPP__
#define __HAL_PD_RAWRCB_HPP__

#include "nic/include/base.h"
#include "nic/utils/ht/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"

using hal::utils::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_RAWRCBS                        4

#define P4PD_RAWRCB_STAGE_ENTRY_OFFSET            64
#define P4PD_HBM_RAWR_CB_ENTRY_SIZE               128

typedef enum rawrcb_hwid_order_ {
    P4PD_HWID_RAWR_TX_STAGE0 = 0,
    P4PD_HWID_RAWR_RX_STAGE0,
} rawrcb_hwid_order_t;

typedef uint64_t    rawrcb_hw_id_t;

// rawrcb pd state
struct pd_rawrcb_s {
    rawrcb_t           *rawrcb;             // PI RAWR CB

    // operational state of rawrcb pd
    rawrcb_hw_id_t     hw_id;               // hw id for this rawrcb

    // meta data maintained for RAWR CB pd
    ht_ctxt_t          hw_ht_ctxt;          // h/w id based hash table ctxt
} __PACK__;

// allocate a rawrcb pd instance
static inline pd_rawrcb_t *
rawrcb_pd_alloc (void)
{
    pd_rawrcb_t    *rawrcb_pd;

    rawrcb_pd = (pd_rawrcb_t *)g_hal_state_pd->rawrcb_slab()->alloc();
    if (rawrcb_pd == NULL) {
        return NULL;
    }

    return rawrcb_pd;
}

// initialize a rawrcb pd instance
static inline pd_rawrcb_t *
rawrcb_pd_init (pd_rawrcb_t *rawrcb_pd)
{
    if (!rawrcb_pd) {
        return NULL;
    }
    rawrcb_pd->rawrcb = NULL;

    // initialize meta information
    rawrcb_pd->hw_ht_ctxt.reset();

    return rawrcb_pd;
}

// allocate and initialize a rawrcb pd instance
static inline pd_rawrcb_t *
rawrcb_pd_alloc_init (void)
{
    return rawrcb_pd_init(rawrcb_pd_alloc());
}

// free rawrcb pd instance
static inline hal_ret_t
rawrcb_pd_free (pd_rawrcb_t *rawrcb_pd)
{
    g_hal_state_pd->rawrcb_slab()->free(rawrcb_pd);
    return HAL_RET_OK;
}

// insert rawrcb pd state in all meta data structures
static inline hal_ret_t
add_rawrcb_pd_to_db (pd_rawrcb_t *rawrcb_pd)
{
    g_hal_state_pd->rawrcb_hwid_ht()->insert(rawrcb_pd, &rawrcb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

static inline hal_ret_t
del_rawrcb_pd_from_db(pd_rawrcb_t *rawrcb_pd)
{
    g_hal_state_pd->rawrcb_hwid_ht()->remove(&rawrcb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a rawrcb pd instance given its hw id
static inline pd_rawrcb_t *
find_rawrcb_by_hwid (rawrcb_hw_id_t hwid)
{
    return (pd_rawrcb_t *)g_hal_state_pd->rawrcb_hwid_ht()->lookup(&hwid);
}

extern void *rawrcb_pd_get_hw_key_func(void *entry);
extern uint32_t rawrcb_pd_compute_hw_hash_func(void *key, uint32_t ht_size);
extern bool rawrcb_pd_compare_hw_key_func(void *key1, void *key2);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_RAWRCB_HPP__

