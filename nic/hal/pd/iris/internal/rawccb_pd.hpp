#ifndef __HAL_PD_RAWCCB_HPP__
#define __HAL_PD_RAWCCB_HPP__

#include "nic/include/base.hpp"
#include "lib/ht/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/app_redir_shared.h"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_RAWCCB_HT_SIZE                 1024
#define P4PD_HBM_RAWCCB_ENTRY_SIZE                RAWCCB_TABLE_ENTRY_SIZE

typedef uint32_t    rawccb_hw_id_t;         // same as rawccb_id_t
typedef uint64_t    rawccb_hw_addr_t;

// rawccb pd state
struct pd_rawccb_s {
    rawccb_t           *rawccb;             // PI RAWC CB

    // operational state of rawccb pd
    rawccb_hw_addr_t   hw_addr;             // hw address for this rawccb
    rawccb_hw_id_t     hw_id;

    // meta data maintained for RAWC CB pd
    ht_ctxt_t          hw_ht_ctxt;          // h/w id based hash table ctxt
} __PACK__;

// allocate a rawccb pd instance
static inline pd_rawccb_t *
rawccb_pd_alloc (void)
{
    pd_rawccb_t    *rawccb_pd;

    rawccb_pd = (pd_rawccb_t *)g_hal_state_pd->rawccb_slab()->alloc();
    if (rawccb_pd == NULL) {
        return NULL;
    }

    return rawccb_pd;
}

// initialize a rawccb pd instance
static inline pd_rawccb_t *
rawccb_pd_init (pd_rawccb_t *rawccb_pd,
                rawccb_hw_id_t hw_id)
{
    if (!rawccb_pd) {
        return NULL;
    }
    rawccb_pd->hw_id = hw_id;
    rawccb_pd->rawccb = NULL;

    // initialize meta information
    rawccb_pd->hw_ht_ctxt.reset();

    return rawccb_pd;
}

// allocate and initialize a rawccb pd instance
static inline pd_rawccb_t *
rawccb_pd_alloc_init (rawccb_hw_id_t hw_id)
{
    return rawccb_pd_init(rawccb_pd_alloc(), hw_id);
}

// free rawccb pd instance
static inline hal_ret_t
rawccb_pd_free (pd_rawccb_t *rawccb_pd)
{
    hal::delay_delete_to_slab(HAL_SLAB_RAWCCB_PD, rawccb_pd);
    return HAL_RET_OK;
}

// insert rawccb pd state in all meta data structures
static inline hal_ret_t
add_rawccb_pd_to_db (pd_rawccb_t *rawccb_pd)
{
    g_hal_state_pd->rawccb_hwid_ht()->insert(rawccb_pd, &rawccb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

static inline hal_ret_t
del_rawccb_pd_from_db(pd_rawccb_t *rawccb_pd)
{
    g_hal_state_pd->rawccb_hwid_ht()->remove_entry(rawccb_pd, &rawccb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a rawccb pd instance given its hw id
static inline pd_rawccb_t *
find_rawccb_by_hwid (rawccb_hw_id_t hwid)
{
    return (pd_rawccb_t *)g_hal_state_pd->rawccb_hwid_ht()->lookup(&hwid);
}

extern void *rawccb_pd_get_hw_key_func(void *entry);
extern uint32_t rawccb_pd_hw_key_size(void);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_RAWCCB_HPP__

