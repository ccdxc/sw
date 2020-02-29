#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/firewall/nwsec_pd.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/src/export/nwsec_api.hpp"

namespace hal {
namespace pd {

//-----------------------------------------------------------------------------
// PD Nwsec Create
//-----------------------------------------------------------------------------
hal_ret_t
pd_nwsec_profile_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t            ret = HAL_RET_OK;
    pd_nwsec_profile_create_args_t *args = pd_func_args->pd_nwsec_profile_create;
    pd_nwsec_profile_t   *pd_nwsec;

    HAL_TRACE_DEBUG("{}: creating pd state ",
                    __FUNCTION__);

    // Create nwsec PD
    pd_nwsec = nwsec_pd_alloc_init();
    if (pd_nwsec == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    nwsec_link_pi_pd(pd_nwsec, args->nwsec_profile);

    // Allocate Resources
    ret = nwsec_pd_alloc_res(pd_nwsec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: Unable to alloc. resources",
                      __FUNCTION__);
        goto end;
    }

    // Program HW
    ret = nwsec_pd_program_hw(pd_nwsec, true, false);

end:
    if (ret != HAL_RET_OK) {
        nwsec_pd_cleanup(pd_nwsec);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD Nwsec Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_nwsec_profile_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t            ret = HAL_RET_OK;;
    pd_nwsec_profile_update_args_t *args = pd_func_args->pd_nwsec_profile_update;
    pd_nwsec_profile_t   *pd_nwsec;

    HAL_TRACE_DEBUG("{}: updating pd state ",
                    __FUNCTION__);

    pd_nwsec = (pd_nwsec_profile_t *)args->clone_profile->pd;
    ret = nwsec_pd_program_hw(pd_nwsec, false, false);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("{}: unable to program hw, ret : {}",
                      __FUNCTION__, ret);
    }

    return ret;

#if 0
    pd_nwsec = (pd_nwsec_profile_t *) args->nwsec_profile->pd;
    // Cache the PI pointer since the ptr in the
    // args is a local copy
    void *cached_pi_ptr = pd_nwsec->nwsec_profile;

    pd_nwsec->nwsec_profile = (void *) args->nwsec_profile;
    // Program HW
    ret = nwsec_pd_program_hw(pd_nwsec, FALSE);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-Nwsec::{}: Unable to program hw, ret : {}",
                      __FUNCTION__, ret);
    }
    // Revert back to the cached PI ptr
    pd_nwsec->nwsec_profile = cached_pi_ptr;
#endif
}

//-----------------------------------------------------------------------------
// PD Nwsec Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_nwsec_profile_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_nwsec_profile_delete_args_t *args = pd_func_args->pd_nwsec_profile_delete;
    pd_nwsec_profile_t  *nwsec_pd;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->nwsec_profile != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->nwsec_profile->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("{}:deleting pd state for nwsec profile {}",
                    __FUNCTION__, args->nwsec_profile->profile_id);
    nwsec_pd = (pd_nwsec_profile_t *)args->nwsec_profile->pd;

    // deprogram HW
    ret = nwsec_pd_deprogram_hw(nwsec_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:unable to deprogram hw", __FUNCTION__);
    }

    // dealloc resources and free
    ret = nwsec_pd_cleanup(nwsec_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed pd nwsec delete",
                      __FUNCTION__);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// pd nwsec profile get pd data
//-----------------------------------------------------------------------------
hal_ret_t
pd_nwsec_profile_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t       ret = HAL_RET_OK;
    pd_nwsec_profile_get_args_t *args = pd_func_args->pd_nwsec_profile_get;
    nwsec_profile_t *nwsec = args->nwsec_profile;
    pd_nwsec_profile_t  *nwsec_pd = (pd_nwsec_profile_t *)nwsec->pd;
    SecurityProfileGetResponse *rsp = args->rsp;

    auto nwsec_info = rsp->mutable_status()->mutable_epd_status();
    nwsec_info->set_hw_sec_profile_id(nwsec_pd->nwsec_hw_id);

    return ret;
}

//-----------------------------------------------------------------------------
// pd nwsec profile restore pd data
//-----------------------------------------------------------------------------
hal_ret_t
pd_nwsec_profile_restore_data (pd_nwsec_profile_restore_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    nwsec_profile_t *nwsec = args->nwsec_profile;
    pd_nwsec_profile_t  *nwsec_pd = (pd_nwsec_profile_t *)nwsec->pd;

    auto nwsec_info = args->status->epd_status();
    nwsec_pd->nwsec_hw_id = nwsec_info.hw_sec_profile_id();

    return ret;
}

//-----------------------------------------------------------------------------
// pd nwsec profile restore
//-----------------------------------------------------------------------------
hal_ret_t
pd_nwsec_profile_restore (pd_func_args_t *pd_func_args)
{
    hal_ret_t           ret;
    pd_nwsec_profile_restore_args_t *args = pd_func_args->pd_nwsec_profile_restore;
    pd_nwsec_profile_t  *nwsec_pd;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("Restoring pd state for vrf {}",
                    args->nwsec_profile->profile_id);

    // allocate PD vrf state
    nwsec_pd = nwsec_pd_alloc_init();
    if (nwsec_pd == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // link pi & pd
    nwsec_link_pi_pd(nwsec_pd, args->nwsec_profile);

    ret = pd_nwsec_profile_restore_data(args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to restore pd data for nwsec prof:{}, err: {}",
                      args->nwsec_profile->profile_id, ret);
        goto end;
    }

    ret = nwsec_pd_program_hw(nwsec_pd, true, true);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to program hw");
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        nwsec_pd_cleanup(nwsec_pd);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram HW
// ----------------------------------------------------------------------------
hal_ret_t
nwsec_pd_deprogram_hw (pd_nwsec_profile_t *nwsec_pd)
{
    hal_ret_t            ret = HAL_RET_OK;

    // Program Input properties Table
    ret = nwsec_pd_depgm_l4_prof_tbl(nwsec_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:unable to deprogram hw", __FUNCTION__);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram l4 profile table
// ----------------------------------------------------------------------------
hal_ret_t
nwsec_pd_depgm_l4_prof_tbl (pd_nwsec_profile_t *nwsec_pd)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    directmap                   *dm;

    dm = g_hal_state_pd->dm_table(P4TBL_ID_L4_PROFILE);
    SDK_ASSERT_RETURN((dm != NULL), HAL_RET_ERR);

    sdk_ret = dm->remove(nwsec_pd->nwsec_hw_id);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:unable to deprogram l4_profile_entry:{},"
                " ret:{}",
                __FUNCTION__, nwsec_pd->nwsec_hw_id, ret);
    } else {
        HAL_TRACE_DEBUG("{}:deprogrammed l4_profile_entry: {}",
                __FUNCTION__, nwsec_pd->nwsec_hw_id);
    }

    return ret;
}


hal_ret_t
nwsec_pd_pgm_l4_profile_table (pd_nwsec_profile_t *pd_nw, bool create,
                               bool is_upgrade)
{
    hal_ret_t                ret;
    sdk_ret_t                sdk_ret;
    directmap                *dm;
    l4_profile_actiondata_t    data = { 0 };

    nwsec_profile_t *profile = (hal::nwsec_profile_t*)pd_nw->nwsec_profile;

    dm = g_hal_state_pd->dm_table(P4TBL_ID_L4_PROFILE);
    SDK_ASSERT(dm != NULL);

    data.action_u.l4_profile_l4_profile.ip_normalization_en =
        profile->ip_normalization_en;
    data.action_u.l4_profile_l4_profile.tcp_normalization_en =
        profile->tcp_normalization_en;
    data.action_u.l4_profile_l4_profile.icmp_normalization_en =
        profile->icmp_normalization_en;

    data.action_u.l4_profile_l4_profile.tcp_split_handshake_drop =
        profile->tcp_split_handshake_drop;
    data.action_u.l4_profile_l4_profile.ip_ttl_change_detect_en =
        profile->ip_ttl_change_detect_en;
    data.action_u.l4_profile_l4_profile.tcp_non_syn_first_pkt_drop =
        profile->tcp_non_syn_first_pkt_drop;
    data.action_u.l4_profile_l4_profile.ip_fragment_drop =
        TRUE;
    data.action_u.l4_profile_l4_profile.ip_rsvd_flags_action =
        profile->ip_rsvd_flags_action;
    data.action_u.l4_profile_l4_profile.ip_df_action =
        profile->ip_df_action;
    data.action_u.l4_profile_l4_profile.ip_options_action =
        profile->ip_options_action;
    data.action_u.l4_profile_l4_profile.ip_invalid_len_action =
        profile->ip_invalid_len_action;
    data.action_u.l4_profile_l4_profile.ip_normalize_ttl =
        profile->ip_normalize_ttl;
    data.action_u.l4_profile_l4_profile.icmp_deprecated_msgs_drop =
        profile->icmp_deprecated_msgs_drop;
    data.action_u.l4_profile_l4_profile.icmp_redirect_msg_drop =
        profile->icmp_redirect_msg_drop;
    data.action_u.l4_profile_l4_profile.icmp_invalid_code_action =
        profile->icmp_invalid_code_action;
    data.action_u.l4_profile_l4_profile.tcp_rsvd_flags_action =
        profile->tcp_rsvd_flags_action;
    data.action_u.l4_profile_l4_profile.tcp_unexpected_mss_action =
        profile->tcp_unexpected_mss_action;
    data.action_u.l4_profile_l4_profile.tcp_unexpected_win_scale_action =
        profile->tcp_unexpected_win_scale_action;
    data.action_u.l4_profile_l4_profile.tcp_unexpected_sack_perm_action =
        profile->tcp_unexpected_sack_perm_action;
    data.action_u.l4_profile_l4_profile.tcp_urg_ptr_not_set_action =
        profile->tcp_urg_ptr_not_set_action;
    data.action_u.l4_profile_l4_profile.tcp_urg_flag_not_set_action =
        profile->tcp_urg_flag_not_set_action;
    data.action_u.l4_profile_l4_profile.tcp_urg_payload_missing_action =
        profile->tcp_urg_payload_missing_action;
    data.action_u.l4_profile_l4_profile.tcp_unexpected_echo_ts_action =
        profile->tcp_unexpected_echo_ts_action;
    data.action_u.l4_profile_l4_profile.tcp_rst_with_data_action =
        profile->tcp_rst_with_data_action;
    data.action_u.l4_profile_l4_profile.tcp_data_len_gt_mss_action =
        profile->tcp_data_len_gt_mss_action;
    data.action_u.l4_profile_l4_profile.tcp_data_len_gt_win_size_action =
        profile->tcp_data_len_gt_win_size_action;
    data.action_u.l4_profile_l4_profile.tcp_unexpected_ts_option_action =
        profile->tcp_unexpected_ts_option_action;
    data.action_u.l4_profile_l4_profile.tcp_unexpected_sack_option_action =
        profile->tcp_unexpected_sack_option_action;
    data.action_u.l4_profile_l4_profile.tcp_ts_not_present_drop =
        profile->tcp_ts_not_present_drop;
    data.action_u.l4_profile_l4_profile.tcp_invalid_flags_drop =
        profile->tcp_invalid_flags_drop;
    data.action_u.l4_profile_l4_profile.tcp_flags_nonsyn_noack_drop =
        profile->tcp_nonsyn_noack_drop;
    data.action_u.l4_profile_l4_profile.tcp_normalize_mss =
        profile->tcp_normalize_mss;
    data.action_u.l4_profile_l4_profile.policy_enf_cfg_en = profile->policy_enforce_en;
    data.action_u.l4_profile_l4_profile.flow_learn_cfg_en = profile->flow_learn_en;
#if 0
    if (g_hal_state->forwarding_mode() == sdk::lib::FORWARDING_MODE_HOSTPIN) {
        data.action_u.l4_profile_l4_profile.policy_enf_cfg_en = 1;
    }
#endif

    if (create) {
        if (is_upgrade) {
            sdk_ret = dm->insert_withid(&data, (uint32_t)pd_nw->nwsec_hw_id);
        } else {
            sdk_ret = dm->insert(&data, (uint32_t *)&pd_nw->nwsec_hw_id);
        }
    } else {
        sdk_ret = dm->update(pd_nw->nwsec_hw_id, &data);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:unable to programm l4_profile_entry: {}, ret:{}",
                      __FUNCTION__, pd_nw->nwsec_hw_id, ret);
    } else {
        HAL_TRACE_DEBUG("{}:programmed l4_profile_entry: {}",
                        __FUNCTION__, pd_nw->nwsec_hw_id);
    }
    return (ret);
}

// ----------------------------------------------------------------------------
// Allocate resources for PD Uplink if
// ----------------------------------------------------------------------------
hal_ret_t
nwsec_pd_alloc_res(pd_nwsec_profile_t *pd_nw)
{
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// De-Allocate resources for PD Uplink if
// ----------------------------------------------------------------------------
hal_ret_t
nwsec_pd_dealloc_res(pd_nwsec_profile_t *pd_nw)
{
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// PD L2seg Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD Vrf
//  Note:
//      - Just free up whatever PD has.
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
hal_ret_t
nwsec_pd_cleanup(pd_nwsec_profile_t *nwsec_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!nwsec_pd) {
        // Nothing to do
        goto end;
    }

    // Releasing resources
    ret = nwsec_pd_dealloc_res(nwsec_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: unable to dealloc res for nwsec: {}",
                      __FUNCTION__,
                      ((nwsec_profile_t*)(nwsec_pd->nwsec_profile))->profile_id);
        goto end;
    }

    // Delinking PI<->PD
    nwsec_delink_pi_pd(nwsec_pd, (nwsec_profile_t *)nwsec_pd->nwsec_profile);

    // Freeing PD
    nwsec_pd_free(nwsec_pd);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
nwsec_pd_program_hw(pd_nwsec_profile_t *pd_nw, bool create, bool is_upgrade)
{
    hal_ret_t            ret = HAL_RET_OK;

    // Program L4 Profile Table
    ret = nwsec_pd_pgm_l4_profile_table(pd_nw, create, is_upgrade);

    return ret;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void
nwsec_link_pi_pd(pd_nwsec_profile_t *pd_nw, nwsec_profile_t *pi_nw)
{
    pd_nw->nwsec_profile = pi_nw;
    nwsec_set_pd_nwsec(pi_nw, pd_nw);
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void
nwsec_delink_pi_pd(pd_nwsec_profile_t *pd_nw, nwsec_profile_t  *pi_nw)
{
    pd_nw->nwsec_profile = NULL;
    nwsec_set_pd_nwsec(pi_nw, NULL);
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
// pd_nwsec_profile_make_clone(nwsec_profile_t *nwsec, nwsec_profile_t *clone)
pd_nwsec_profile_make_clone(pd_func_args_t *pd_func_args)
{
    hal_ret_t           ret             = HAL_RET_OK;
    pd_nwsec_profile_make_clone_args_t *args = pd_func_args->pd_nwsec_profile_make_clone;
    pd_nwsec_profile_t  *pd_nwsec_clone = NULL;
    nwsec_profile_t     *nwsec          = args->nwsec_profile;
    nwsec_profile_t     *clone          = args->clone_profile;

    pd_nwsec_clone = nwsec_pd_alloc_init();
    if (pd_nwsec_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_nwsec_clone, nwsec->pd, sizeof(pd_nwsec_profile_t));

    nwsec_link_pi_pd(pd_nwsec_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_nwsec_profile_mem_free(pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd_nwsec_profile_mem_free_args_t *args = pd_func_args->pd_nwsec_profile_mem_free;
    pd_nwsec_profile_t      *nwsec_pd;

    nwsec_pd = (pd_nwsec_profile_t *)args->nwsec_profile->pd;
    nwsec_pd_mem_free(nwsec_pd);

    return ret;
}



}    // namespace pd
}    // namespace hal
