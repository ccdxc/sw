#ifndef __HAL_PD_WRING_HPP__
#define __HAL_PD_WRING_HPP__

#include "nic/include/pd.hpp"
#include "gen/proto/internal.pb.h"
#include "nic/hal/src/internal/wring.hpp"
#include "nic/include/base.hpp"
#include "lib/ht/ht.hpp"
//#include "nic/hal/pd/iris/hal_state_pd.hpp"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_WRING                        2048
#define DEFAULT_WRING_SLOT_SIZE                 8 

#define SERQ_WRING_SLOT_SIZE                    (32)
#define BSQ_WRING_SLOT_SIZE                     (32)

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
typedef hal_ret_t (*wring_set_hw_meta)(pd_wring_t* wring_pd);

#define MAX_WRING_IDS 16 // CPU Cores/threads

struct pd_wring_meta_s {
    bool        is_global;
    char        hbm_reg_name[64];
    uint32_t    num_slots;
    uint32_t    slot_size_in_bytes;
    char        obj_hbm_reg_name[64];
    uint32_t    obj_size;
    uint64_t    alloc_semaphore_addr;
    wring_slot_parser   slot_parser;
    wring_get_hw_meta   get_hw_meta_fn;   
    bool        skip_init_slots;
    uint32_t    ring_types_in_region;
    uint32_t    ring_type_offset;
    bool        mmap_ring; // Whether ring to be memmap'ed for direct access
    bool        per_flow_ring;
    wring_hw_id_t base_addr[MAX_WRING_IDS];
    wring_hw_id_t obj_base_addr[MAX_WRING_IDS];
    uint8_t     *virt_base_addr[MAX_WRING_IDS]; // Virtual address of the ring base in memory
    uint8_t     *virt_obj_base_addr[MAX_WRING_IDS]; // Virt addr of the object ring base in memory
    wring_set_hw_meta   set_hw_meta_fn;   
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

// Get the num_entries for the ring
hal_ret_t wring_pd_get_num_entries(types::WRingType type,
                                   uint32_t *num_entries);

pd_wring_meta_t* wring_pd_get_meta(types::WRingType type);
hal_ret_t wring_pd_table_init(types::WRingType type, uint32_t wring_id); 

extern void *wring_pd_get_hw_key_func(void *entry);
extern uint32_t wring_pd_hw_key_size(void);

#define WRING_PD_META_SET_BASE_ADDR(meta_, wring_id_, obj_, val_)       \
    if (!meta_->per_flow_ring && (wring_id_ < MAX_WRING_IDS)) {         \
        meta_->obj_[wring_id_] = val_;                                  \
    }                                                                   \
    
#define WRING_PD_META_GET_BASE_ADDR(meta_, wring_id_, obj_, null_val_)  \
    return !meta_->per_flow_ring && (wring_id_ < MAX_WRING_IDS) ?       \
            meta_->obj_[wring_id_] : null_val_;                         \
    
static inline void
wring_pd_set_meta_base_addr(pd_wring_meta_t *meta,
                            uint32_t wring_id,
                            wring_hw_id_t val)
{
    WRING_PD_META_SET_BASE_ADDR(meta, wring_id, base_addr, val);
}

static inline wring_hw_id_t
wring_pd_get_meta_base_addr(pd_wring_meta_t *meta,
                            uint32_t wring_id)
{
    WRING_PD_META_GET_BASE_ADDR(meta, wring_id, base_addr, 0);
}

static inline void
wring_pd_set_meta_obj_base_addr(pd_wring_meta_t *meta,
                                uint32_t wring_id,
                                wring_hw_id_t val)
{
    WRING_PD_META_SET_BASE_ADDR(meta, wring_id, obj_base_addr, val);
}

static inline wring_hw_id_t
wring_pd_get_meta_obj_base_addr(pd_wring_meta_t *meta,
                                uint32_t wring_id)
{
    WRING_PD_META_GET_BASE_ADDR(meta, wring_id, obj_base_addr, 0);
}

static inline void
wring_pd_set_meta_virt_base_addr(pd_wring_meta_t *meta,
                                 uint32_t wring_id,
                                 uint8_t *val)
{
    WRING_PD_META_SET_BASE_ADDR(meta, wring_id, virt_base_addr, val);
}

static inline uint8_t *
wring_pd_get_meta_virt_base_addr(pd_wring_meta_t *meta,
                                 uint32_t wring_id)
{
    WRING_PD_META_GET_BASE_ADDR(meta, wring_id, virt_base_addr, nullptr);
}

static inline void
wring_pd_set_meta_virt_obj_base_addr(pd_wring_meta_t *meta,
                                     uint32_t wring_id,
                                     uint8_t *val)
{
    WRING_PD_META_SET_BASE_ADDR(meta, wring_id, virt_obj_base_addr, val);
}

static inline uint8_t *
wring_pd_get_meta_virt_obj_base_addr(pd_wring_meta_t *meta,
                                     uint32_t wring_id)
{
    WRING_PD_META_GET_BASE_ADDR(meta, wring_id, virt_obj_base_addr, nullptr);
}

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_WRING_HPP__

