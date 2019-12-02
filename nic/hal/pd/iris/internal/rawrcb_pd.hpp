#ifndef __HAL_PD_RAWRCB_HPP__
#define __HAL_PD_RAWRCB_HPP__

#include "nic/include/base.hpp"
#include "lib/ht/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/app_redir_shared.h"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_RAWRCB_HT_SIZE       1024
#define P4PD_HBM_RAWRCB_ENTRY_SIZE      RAWRCB_TABLE_ENTRY_SIZE

typedef uint32_t    rawrcb_hw_id_t;         // same as rawrcb_id_t
typedef uint64_t    rawrcb_hw_addr_t;

// rawrcb pd state
struct pd_rawrcb_s {
    rawrcb_t           *rawrcb;             // PI RAWR CB

    // operational state of rawrcb pd
    rawrcb_hw_addr_t   hw_addr;             // hw address for this rawrcb
    rawrcb_hw_id_t     hw_id;

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
rawrcb_pd_init (pd_rawrcb_t *rawrcb_pd,
                rawrcb_hw_id_t hw_id)
{
    if (!rawrcb_pd) {
        return NULL;
    }
    rawrcb_pd->hw_id = hw_id;
    rawrcb_pd->rawrcb = NULL;

    // initialize meta information
    rawrcb_pd->hw_ht_ctxt.reset();

    return rawrcb_pd;
}

// allocate and initialize a rawrcb pd instance
static inline pd_rawrcb_t *
rawrcb_pd_alloc_init (rawrcb_hw_id_t hw_id)
{
    return rawrcb_pd_init(rawrcb_pd_alloc(), hw_id);
}

// free rawrcb pd instance
static inline hal_ret_t
rawrcb_pd_free (pd_rawrcb_t *rawrcb_pd)
{
    hal::delay_delete_to_slab(HAL_SLAB_RAWRCB_PD, rawrcb_pd);
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
    g_hal_state_pd->rawrcb_hwid_ht()->remove_entry(rawrcb_pd, &rawrcb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a rawrcb pd instance given its hw id
static inline pd_rawrcb_t *
find_rawrcb_by_hwid (rawrcb_hw_id_t hwid)
{
    return (pd_rawrcb_t *)g_hal_state_pd->rawrcb_hwid_ht()->lookup(&hwid);
}

extern void *rawrcb_pd_get_hw_key_func(void *entry);
extern uint32_t rawrcb_pd_hw_key_size(void);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_RAWRCB_HPP__

