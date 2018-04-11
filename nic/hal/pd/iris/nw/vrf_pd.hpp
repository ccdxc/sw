// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_PD_VRF_HPP__
#define __HAL_PD_VRF_HPP__

#include "sdk/ht.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/base.h"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/nw/l2seg_pd.hpp"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_VRFS         256
#define HAL_PD_L2SEG_MASK       0xFFF
#define HAL_PD_VRF_MASK         0xF000
#define HAL_PD_VRF_SHIFT        12
#define HAL_PD_INP_MAP_ENTRIES  3        /* Mac, IPv4, IPv6 */

typedef uint32_t    vrf_hw_id_t;

// vrf pd state
struct pd_vrf_s {
    uint32_t           gipo_imn_idx[3];       // Imapping native table idx
    uint32_t           gipo_imt_idx[3];       // Imapping tunneled table idx
    vrf_hw_id_t        vrf_hw_id;             // hw id for this VRF
    uint32_t           vrf_fl_lkup_id;        // used by IPSec for flow lookup
    uint32_t           vrf_fromcpu_vlan_id;   // from CPU vlan id
    indexer            *l2seg_hw_id_idxr_;    // indexer for l2segs
    uint32_t           inp_prop_tbl_cpu_idx;  // from CPU traffic

    void               *vrf;                  // PI vrf
} __PACK__;

// allocate a vrf pd instance
static inline pd_vrf_t *
vrf_pd_alloc (void)
{
    pd_vrf_t    *vrf_pd;

    vrf_pd = (pd_vrf_t *)g_hal_state_pd->vrf_slab()->alloc();
    if (vrf_pd == NULL) {
        return NULL;
    }

    return vrf_pd;
}

// initialize a vrf pd instance
static inline pd_vrf_t *
vrf_pd_init (pd_vrf_t *vrf_pd)
{
    if (!vrf_pd) {
        return NULL;
    }
    vrf_pd->vrf                  = NULL;
    vrf_pd->vrf_hw_id            = INVALID_INDEXER_INDEX;
    vrf_pd->vrf_fl_lkup_id       = INVALID_INDEXER_INDEX;
    vrf_pd->vrf_fromcpu_vlan_id  = INVALID_INDEXER_INDEX;
    vrf_pd->inp_prop_tbl_cpu_idx = INVALID_INDEXER_INDEX;
    for (int i = 0; i < 3; i++) {
        vrf_pd->gipo_imn_idx[i] = INVALID_INDEXER_INDEX;
        vrf_pd->gipo_imt_idx[i] = INVALID_INDEXER_INDEX;
    }

    vrf_pd->l2seg_hw_id_idxr_ =
        sdk::lib::indexer::factory(HAL_MAX_HW_L2SEGMENTS, true, true);
    HAL_ASSERT_RETURN((vrf_pd->l2seg_hw_id_idxr_ != NULL), NULL);

    // prevention of usage of 0
    vrf_pd->l2seg_hw_id_idxr_->alloc_withid(0);

    return vrf_pd;
}

// allocate and initialize a vrf pd instance
static inline pd_vrf_t *
vrf_pd_alloc_init (void)
{
    return vrf_pd_init(vrf_pd_alloc());
}

// freeing vrf pd
static inline hal_ret_t
vrf_pd_free (pd_vrf_t *ten)
{
    ten->l2seg_hw_id_idxr_ ? indexer::destroy(ten->l2seg_hw_id_idxr_) : HAL_NOP;
    hal::pd::delay_delete_to_slab(HAL_SLAB_VRF_PD, ten);
    return HAL_RET_OK;
}

// freeing vrf pd memory
static inline hal_ret_t
vrf_pd_mem_free (pd_vrf_t *ten)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_VRF_PD, ten);
    return HAL_RET_OK;
}

// l2seg hw id indexer
static inline indexer *
vrf_pd_l2seg_hw_id_indexer(pd_vrf_t *vrf_pd)
{
    return vrf_pd->l2seg_hw_id_idxr_;

}

// exported APIs
hal_ret_t vrf_pd_free_l2seg_hw_id(pd_vrf_t *vrf_pd, uint32_t l2seg_hw_id);
hal_ret_t vrf_pd_alloc_l2seg_hw_id(pd_vrf_t *vrf_pd, uint32_t *l2seg_hw_id);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_VRF_HPP__
