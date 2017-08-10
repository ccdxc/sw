#include <base.h>
#include <hal_lock.hpp>
#include <p4pd.h>
#include <pd_api.hpp>
#include <nwsec_pd.hpp>
#include <pd.hpp>
#include <hal_state_pd.hpp>
#include <nwsec_api.hpp>

namespace hal {
namespace pd {

hal_ret_t
p4pd_program_l4_profile_table (pd_nwsec_profile_t *pd_nw, bool create)
{
    hal_ret_t                ret;
    DirectMap                *dm;
    l4_profile_actiondata    data = { 0 };

    nwsec_profile_t *profile = (hal::nwsec_profile_t*)pd_nw->nwsec_profile;

    dm = g_hal_state_pd->dm_table(P4TBL_ID_L4_PROFILE);
    HAL_ASSERT(dm != NULL);

    data.l4_profile_action_u.l4_profile_l4_profile.ip_normalization_en =
        profile->ip_normalization_en;
    data.l4_profile_action_u.l4_profile_l4_profile.tcp_normalization_en =
        profile->tcp_normalization_en;
    data.l4_profile_action_u.l4_profile_l4_profile.icmp_normalization_en =
        profile->icmp_normalization_en;
    data.l4_profile_action_u.l4_profile_l4_profile.tcp_split_handshake_detect_en =
        profile->tcp_split_handshake_detect_en;
    data.l4_profile_action_u.l4_profile_l4_profile.tcp_split_handshake_drop =
        profile->tcp_split_handshake_drop;
    data.l4_profile_action_u.l4_profile_l4_profile.ip_ttl_change_detect_en =
        profile->ip_ttl_change_detect_en;
    data.l4_profile_action_u.l4_profile_l4_profile.tcp_non_syn_first_pkt_drop =
        profile->tcp_non_syn_first_pkt_drop;
    data.l4_profile_action_u.l4_profile_l4_profile.ip_fragment_drop =
        TRUE;
    data.l4_profile_action_u.l4_profile_l4_profile.ip_rsvd_flags_action =
        profile->ip_rsvd_flags_action;
    data.l4_profile_action_u.l4_profile_l4_profile.ip_df_action =
        profile->ip_df_action;
    data.l4_profile_action_u.l4_profile_l4_profile.ip_options_action =
        profile->ip_options_action;
    data.l4_profile_action_u.l4_profile_l4_profile.ip_invalid_len_action =
        profile->ip_invalid_len_action;
    data.l4_profile_action_u.l4_profile_l4_profile.icmp_deprecated_msgs_drop =
        profile->icmp_deprecated_msgs_drop;
    data.l4_profile_action_u.l4_profile_l4_profile.icmp_redirect_msg_drop =
        profile->icmp_redirect_msg_drop;
    data.l4_profile_action_u.l4_profile_l4_profile.icmp_invalid_code_action =
        profile->icmp_invalid_code_action;
    data.l4_profile_action_u.l4_profile_l4_profile.tcp_rsvd_flags_action =
        profile->tcp_rsvd_flags_action;
    data.l4_profile_action_u.l4_profile_l4_profile.tcp_unexpected_mss_action =
        profile->tcp_unexpected_mss_action;
    data.l4_profile_action_u.l4_profile_l4_profile.tcp_unexpected_win_scale_action =
        profile->tcp_unexpected_win_scale_action;
    data.l4_profile_action_u.l4_profile_l4_profile.tcp_urg_ptr_not_set_action =
        profile->tcp_urg_ptr_not_set_action;
    data.l4_profile_action_u.l4_profile_l4_profile.tcp_urg_flag_not_set_action =
        profile->tcp_urg_flag_not_set_action;
    data.l4_profile_action_u.l4_profile_l4_profile.tcp_urg_payload_missing_action =
        profile->tcp_urg_payload_missing_action;
    data.l4_profile_action_u.l4_profile_l4_profile.tcp_unexpected_echo_ts_action =
        profile->tcp_unexpected_echo_ts_action;
    data.l4_profile_action_u.l4_profile_l4_profile.tcp_rst_with_data_action =
        profile->tcp_rst_with_data_action;
    data.l4_profile_action_u.l4_profile_l4_profile.tcp_data_len_gt_mss_action =
        profile->tcp_data_len_gt_mss_action;
    data.l4_profile_action_u.l4_profile_l4_profile.tcp_data_len_gt_win_size_action =
        profile->tcp_data_len_gt_win_size_action;
    data.l4_profile_action_u.l4_profile_l4_profile.tcp_unexpected_ts_option_action =
        profile->tcp_unexpected_ts_option_action;
    data.l4_profile_action_u.l4_profile_l4_profile.tcp_ts_not_present_drop =
        profile->tcp_ts_not_present_drop;
    data.l4_profile_action_u.l4_profile_l4_profile.tcp_invalid_flags_drop =
        profile->tcp_invalid_flags_drop;
    data.l4_profile_action_u.l4_profile_l4_profile.tcp_flags_nonsyn_noack_drop =
        profile->tcp_flags_nonsyn_noack_drop;
    
    if (create) {
        ret = dm->insert(&data, (uint32_t *)&pd_nw->nwsec_hw_id);
    } else {
        ret = dm->update(pd_nw->nwsec_hw_id, &data);
    }
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-Nwsec::{}: Unable to program L4 Profile: create: {} ret: {}",
                __FUNCTION__, create, ret);
    } else {
        HAL_TRACE_DEBUG("PD-Nwsec::{}: Programmed L4 Profile at {} create {}",
                __FUNCTION__, pd_nw->nwsec_hw_id, create);
    }
    return (ret);
}

hal_ret_t
pd_nwsec_profile_update (pd_nwsec_profile_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;; 
    pd_nwsec_profile_t   *pd_nwsec;

    HAL_TRACE_DEBUG("PD-Nwsec:{}: Updating pd state ",
                    __FUNCTION__);

    pd_nwsec = (pd_nwsec_profile_t *) args->nwsec_profile->pd;
    // Cache the PI pointer since the ptr in the
    // args is a local copy
    void *cached_pi_ptr = pd_nwsec->nwsec_profile;
     
    pd_nwsec->nwsec_profile = (void *) args->nwsec_profile; 
    // Program HW
    ret = pd_nwsec_program_hw(pd_nwsec, FALSE);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-Nwsec::{}: Unable to program hw, ret : {}",
                      __FUNCTION__, ret);
    }
    // Revert back to the cached PI ptr
    pd_nwsec->nwsec_profile = cached_pi_ptr;

    return ret;
}

hal_ret_t
pd_nwsec_profile_create (pd_nwsec_profile_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;; 
    pd_nwsec_profile_t   *pd_nwsec;

    HAL_TRACE_DEBUG("PD-Nwsec:{}: Creating pd state ",
                    __FUNCTION__);

    // Create nwsec PD
    pd_nwsec = pd_nwsec_alloc_init();
    if (pd_nwsec == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    link_pi_pd(pd_nwsec, args->nwsec_profile);

    // Allocate Resources
    ret = pd_nwsec_alloc_res(pd_nwsec);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-Nwsec::{}: Unable to alloc. resources",
                      __FUNCTION__);
        goto end;
    }

    // Program HW
    ret = pd_nwsec_program_hw(pd_nwsec, TRUE);

end:
    if (ret != HAL_RET_OK) {
        unlink_pi_pd(pd_nwsec, args->nwsec_profile);
        pd_nwsec_free(pd_nwsec);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Allocate and Initialize Uplinkif PD Instance
// ----------------------------------------------------------------------------
pd_nwsec_profile_t *
pd_nwsec_alloc_init(void)
{
    return pd_nwsec_init(pd_nwsec_alloc());
}

// ----------------------------------------------------------------------------
// Allocate Uplink IF Instance
// ----------------------------------------------------------------------------
pd_nwsec_profile_t *
pd_nwsec_alloc (void)
{
    pd_nwsec_profile_t    *pd_nw;

    pd_nw = (pd_nwsec_profile_t *)g_hal_state_pd->nwsec_pd_slab()->alloc();
    if (pd_nw == NULL) {
        return NULL;
    }
    return pd_nw;
}

// ----------------------------------------------------------------------------
// Initialize Uplink IF PD instance
// ----------------------------------------------------------------------------
pd_nwsec_profile_t *
pd_nwsec_init (pd_nwsec_profile_t *pd_nw)
{
    // Nothing to do currently
    if (!pd_nw) {
        return NULL;
    }

    // Set here if you want to initialize any fields

    return pd_nw;
}

// ----------------------------------------------------------------------------
// Allocate resources for PD Uplink if
// ----------------------------------------------------------------------------
hal_ret_t 
pd_nwsec_alloc_res(pd_nwsec_profile_t *pd_nw)
{
    return HAL_RET_OK;

#if 0
    // Clean up. Id will be allocated inside DirectMap library
    hal_ret_t            ret = HAL_RET_OK;
    indexer::status      rs = indexer::SUCCESS;

    // Allocate lif hwid
    rs = g_hal_state_pd->nwsec_profile_hwid_idxr()->alloc((uint32_t *)&pd_nw->hw_id);
    if (rs != indexer::SUCCESS) {
        return HAL_RET_NO_RESOURCE;
    }

    HAL_TRACE_DEBUG("PD-Nwsec:{}: Allocated hw_id: {}", 
                    __FUNCTION__, pd_nw->hw_id);
    return ret;
#endif
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
pd_nwsec_program_hw(pd_nwsec_profile_t *pd_nw, bool create)
{
    hal_ret_t            ret = HAL_RET_OK;

    // Program L4 Profile Table
    ret = p4pd_program_l4_profile_table(pd_nw, create);
    // ret = p4pd_program_l4_profile_table((hal::nwsec_profile_t*)pd_nw->nwsec_profile);

    return ret;
}

// ----------------------------------------------------------------------------
// Freeing Nwsec PD
// ----------------------------------------------------------------------------
hal_ret_t
pd_nwsec_free (pd_nwsec_profile_t *pd_nw)
{
    g_hal_state_pd->nwsec_pd_slab()->free(pd_nw);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
link_pi_pd(pd_nwsec_profile_t *pd_nw, nwsec_profile_t *pi_nw)
{
    pd_nw->nwsec_profile = pi_nw;
    nwsec_set_pd_nwsec(pi_nw, pd_nw);
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
unlink_pi_pd(pd_nwsec_profile_t *pd_nw, nwsec_profile_t  *pi_nw)
{
    pd_nw->nwsec_profile = NULL;
    nwsec_set_pd_nwsec(pi_nw, NULL);
}

}    // namespace pd
}    // namespace hal
