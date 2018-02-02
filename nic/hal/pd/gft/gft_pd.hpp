#ifndef __HAL_PD_GFT_HPP__
#define __HAL_PD_GFT_HPP__

#include "nic/include/base.h"
#include "sdk/ht.hpp"
#include "nic/hal/pd/gft/gft_pd.hpp"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_GFT_EXACT_MATCH_PROFILES            512

struct pd_gft_exact_match_profile_s {
    void    *gft_exact_match_profile;        // PI GFT exact match profile
} __PACK__;

// allocate a gft exact match profile pd instance
static inline pd_gft_exact_match_profile_t *
gft_exact_match_profile_pd_alloc (void)
{
    pd_gft_exact_match_profile_t *gft_exact_match_profile_pd = NULL;

#if 0
    gft_exact_match_profile_pd =
        (pd_gft_exact_match_profile_t *)
            g_hal_state_pd->gft_exact_match_profile_slab()->alloc();
#endif
    return gft_exact_match_profile_pd;
}

// initialize a gft exact match profile pd instance
static inline pd_gft_exact_match_profile_t *
gft_exact_match_profile_pd_init (pd_gft_exact_match_profile_t *gft_exact_match_profile_pd)
{
    if (!gft_exact_match_profile_pd) {
        return NULL;
    }
    gft_exact_match_profile_pd->gft_exact_match_profile = NULL;

    return gft_exact_match_profile_pd;
}

// allocate and initialize a gft exact match profile pd instance
static inline pd_gft_exact_match_profile_t *
gft_exact_match_profile_pd_alloc_init (void)
{
    return gft_exact_match_profile_pd_init(gft_exact_match_profile_pd_alloc());
}

// free gft exact match profile pd instance
static inline hal_ret_t
gft_exact_match_profile_pd_free (pd_gft_exact_match_profile_t *gft_exact_match_profile_pd)
{
#if 0
    hal::pd::delay_delete_to_slab(HAL_SLAB_GFT_EXACT_MATCH_PROFILE_PD,
                                  gft_exact_match_profile_pd);
#endif
    return HAL_RET_OK;
}

struct pd_gft_hdr_xposition_profile_s {
    void    *gft_hdr_xposition_profile;    // PI GFT header transposition profile
} __PACK__;

// allocate a gft header transposition profile pd instance
static inline pd_gft_hdr_xposition_profile_t *
gft_hdr_xposition_profile_pd_alloc (void)
{
    pd_gft_hdr_xposition_profile_t *gft_hdr_xposition_profile_pd = NULL;

#if 0
    gft_hdr_xposition_profile_pd =
        (pd_gft_hdr_xposition_profile_t *)
            g_hal_state_pd->gft_hdr_xposition_profile_slab()->alloc();
#endif
    return gft_hdr_xposition_profile_pd;
}

// initialize a gft header transposition profile pd instance
static inline pd_gft_hdr_xposition_profile_t *
gft_hdr_xposition_profile_pd_init (pd_gft_hdr_xposition_profile_t *gft_hdr_xposition_profile_pd)
{
    if (!gft_hdr_xposition_profile_pd) {
        return NULL;
    }
    gft_hdr_xposition_profile_pd->gft_hdr_xposition_profile = NULL;

    return gft_hdr_xposition_profile_pd;
}

// allocate and initialize a gft header transposition profile pd instance
static inline pd_gft_hdr_xposition_profile_t *
gft_hdr_xposition_profile_pd_alloc_init (void)
{
    return gft_hdr_xposition_profile_pd_init(gft_hdr_xposition_profile_pd_alloc());
}

// free gft header transposition profile pd instance
static inline hal_ret_t
gft_hdr_xposition_profile_pd_free (pd_gft_hdr_xposition_profile_t *gft_hdr_xposition_profile_pd)
{
#if 0
    hal::pd::delay_delete_to_slab(HAL_SLAB_GFT_HDR_XPOSITION_PROFILE_PD,
                                  gft_hdr_xposition_profile_pd);
#endif
    return HAL_RET_OK;
}

struct pd_gft_exact_match_flow_entry_s {
    void    *gft_exact_match_flow_entry;    // PI GFT exact match flow entry
} __PACK__;

// allocate a gft exact match flow entry pd instance
static inline pd_gft_exact_match_flow_entry_t *
gft_exact_match_flow_entry_pd_alloc (void)
{
    pd_gft_exact_match_flow_entry_t *gft_exact_match_flow_entry_pd = NULL;

#if 0
    gft_exact_match_flow_entry_pd =
        (pd_gft_exact_match_flow_entry_t *)
            g_hal_state_pd->gft_exact_match_flow_entry_slab()->alloc();
#endif
    return gft_exact_match_flow_entry_pd;
}

// initialize a gft exact match flow entry pd instance
static inline pd_gft_exact_match_flow_entry_t *
gft_exact_match_flow_entry_pd_init (pd_gft_exact_match_flow_entry_t *gft_exact_match_flow_entry_pd)
{
    if (!gft_exact_match_flow_entry_pd) {
        return NULL;
    }
    gft_exact_match_flow_entry_pd->gft_exact_match_flow_entry = NULL;

    return gft_exact_match_flow_entry_pd;
}

// allocate and initialize a gft exact match flow entry pd instance
static inline pd_gft_exact_match_flow_entry_t *
gft_exact_match_flow_entry_pd_alloc_init (void)
{
    return gft_exact_match_flow_entry_pd_init(gft_exact_match_flow_entry_pd_alloc());
}

// free gft exact match flow entry pd instance
static inline hal_ret_t
gft_exact_match_flow_entry_pd_free (pd_gft_exact_match_flow_entry_t *gft_exact_match_flow_entry_pd)
{
#if 0
    hal::pd::delay_delete_to_slab(HAL_SLAB_GFT_EXACT_MATCH_FLOW_ENTRY_PD,
                                  gft_exact_match_flow_entry_pd);
#endif
    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal

#endif    // __HAL_PD_GFT_HPP__

