#ifndef __HAL_PD_WRING_HPP__
#define __HAL_PD_WRING_HPP__

#include "nic/include/pd.hpp"
#include "nic/gen/proto/hal/wring.pb.h"
#include "nic/hal/src/wring.hpp"
#include "nic/include/base.h"
#include "sdk/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_WRING                        2048
#define DEFAULT_WRING_SLOT_SIZE                 8 

#define SERQ_WRING_SLOT_SIZE                    (32)

typedef uint64_t    wring_hw_id_t;

// wring pd state
struct pd_wring_s {
    wring_t           *wring;              // PI TCP CB

    // operational state of wring pd
    wring_hw_id_t      hw_id;               // hw id for this wring

    // meta data maintained for TCP CB pd
    ht_ctxt_t          hw_ht_ctxt;           // h/w id based hash table ctxt
} __PACK__;

typedef hal_ret_t (*wring_slot_parser)(pd_wring_meta_t *meta, wring_t *wring, uint8_t *slot);
typedef hal_ret_t (*wring_get_hw_meta)(pd_wring_t* wring_pd);

struct pd_wring_meta_s {
    bool        is_global;
    char        hbm_reg_name[64];
    uint32_t    num_slots;
    uint32_t    slot_size_in_bytes;
    char        obj_hbm_reg_name[64];
    uint32_t    obj_size;
    uint64_t    alloc_semaphore_addr;
    uint64_t    free_semaphore_addr;
    wring_slot_parser   slot_parser;
    wring_get_hw_meta   get_hw_meta_fn;   
    bool        skip_init_slots;
} __PACK__;

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

hal_ret_t wring_pd_init_global_rings(void);

// Get the base address for the ring
hal_ret_t wring_pd_get_base_addr(types::WRingType type,
                                 uint32_t wring_id,
                                 wring_hw_id_t* wring_base);

pd_wring_meta_t* wring_pd_get_meta(types::WRingType type);
hal_ret_t wring_pd_table_init(types::WRingType type, uint32_t wring_id); 

extern void *wring_pd_get_hw_key_func(void *entry);
extern uint32_t wring_pd_compute_hw_hash_func(void *key, uint32_t ht_size);
extern bool wring_pd_compare_hw_key_func(void *key1, void *key2);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_WRING_HPP__

