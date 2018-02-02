// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/gft.hpp"
#include "nic/hal/pd/gft/gft_pd.hpp"

namespace hal {
namespace pd {

//------------------------------------------------------------------------------
// link GFT exact match profile's PI and PD states
//------------------------------------------------------------------------------
static void
gft_exact_match_profile_link_pi_pd (pd_gft_exact_match_profile_t *pd,
                                    gft_exact_match_profile_t *pi)
{
    pd->gft_exact_match_profile = pi;
    pi->pd = pd;
}

//------------------------------------------------------------------------------
// create necessary state for the GFT exact match profile
//------------------------------------------------------------------------------
hal_ret_t
pd_gft_exact_match_profile_create (pd_gft_args_t *gft_args)
{
    hal_ret_t                       ret = HAL_RET_OK;
    pd_gft_exact_match_profile_t    *gft_exact_match_profile_pd = NULL;

    if (!gft_args || !gft_args->exact_match_profile) {
        return HAL_RET_INVALID_ARG;
    }

    if (gft_args->exact_match_profile->profile_id == HAL_GFT_INVALID_PROFILE_ID) {
        return HAL_RET_INVALID_ARG;
    }

    if (!gft_args->exact_match_profile->num_hdr_group_exact_match_profiles) {
        return HAL_RET_INVALID_ARG;
    }

    // create PD state for this exact match profile
    gft_exact_match_profile_pd = gft_exact_match_profile_pd_alloc_init();
    if (gft_exact_match_profile_pd == NULL) {
        return HAL_RET_OOM;
    }

    // link PI and PD states
    gft_exact_match_profile_link_pi_pd(gft_exact_match_profile_pd,
                                       gft_args->exact_match_profile);

#if 0
    ret = gft_exact_match_profile_pd_add_to_db();
    if (ret != HAL_RET_OK) {
        gft_exact_match_profile_pd_cleanup(gft_exact_match_profile_pd);
    }
#endif

    return ret;
}

//------------------------------------------------------------------------------
// link GFT header transposition profile's PI and PD states
//------------------------------------------------------------------------------
static void
gft_hdr_group_xposition_link_pi_pd (pd_gft_hdr_xposition_profile_t *pd,
                                    gft_hdr_xposition_profile_t *pi)
{
    pd->gft_hdr_xposition_profile = pi;
    pi->pd = pd;
}

hal_ret_t
pd_gft_hdr_group_xposition_profile_create (pd_gft_args_t *gft_args)
{
    pd_gft_hdr_xposition_profile_t    *gft_hdr_xposition_profile_pd = NULL;

    if (!gft_args || !gft_args->hdr_xposition_profile) {
        return HAL_RET_INVALID_ARG;
    }

    if (gft_args->hdr_xposition_profile->profile_id == HAL_GFT_INVALID_PROFILE_ID) {
        return HAL_RET_INVALID_ARG;
    }

    if (!gft_args->hdr_xposition_profile->num_hxp) {
        return HAL_RET_INVALID_ARG;
    }

    // create PD state for this exact match profile
    gft_hdr_xposition_profile_pd = gft_hdr_xposition_profile_pd_alloc_init();
    if (gft_hdr_xposition_profile_pd == NULL) {
        return HAL_RET_OOM;
    }

    // link PI and PD states
    gft_hdr_group_xposition_link_pi_pd(gft_hdr_xposition_profile_pd,
                                       gft_args->hdr_xposition_profile);

    return HAL_RET_OK;
}

hal_ret_t
pd_gft_exact_match_flow_entry_create (pd_gft_args_t *gft_args)
{
    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
