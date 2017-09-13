// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <base.h>
#include <hal.hpp>
#include <hal_lock.hpp>
#include <hal_state.hpp>
#include <nwsec.hpp>
#include <nwsec_svc.hpp>
#include <pd_api.hpp>

namespace hal {

void *
nwsec_profile_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((nwsec_profile_t *)entry)->profile_id);
}

uint32_t
nwsec_profile_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key,
                                      sizeof(nwsec_profile_id_t)) % ht_size;
}

bool
nwsec_profile_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(nwsec_profile_id_t *)key1 == *(nwsec_profile_id_t *)key2) {
        return true;
    }
    return false;
}

void *
nwsec_profile_get_handle_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((nwsec_profile_t *)entry)->hal_handle);
}

uint32_t
nwsec_profile_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
nwsec_profile_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

static inline nwsec_profile_t *
nwsec_profile_lookup (const nwsec::SecurityProfileKeyHandle& key_or_handle)
{
    if (key_or_handle.key_or_handle_case() ==
            SecurityProfileKeyHandle::kProfileId) {
        return nwsec_profile_lookup_by_id(key_or_handle.profile_id());
    } else {
        return nwsec_profile_lookup_by_handle(key_or_handle.profile_handle());
    }
    return NULL;
}

// initialize security profile object from the config spec
static inline void
nwsec_profile_init_from_spec (nwsec_profile_t *sec_prof,
                              nwsec::SecurityProfileSpec& spec)
{
    sec_prof->profile_id = spec.key_or_handle().profile_id();
    sec_prof->cnxn_tracking_en = spec.cnxn_tracking_en();
    sec_prof->ipsg_en = spec.ipsg_en();
    sec_prof->tcp_rtt_estimate_en = spec.tcp_rtt_estimate_en();
    sec_prof->session_idle_timeout = spec.session_idle_timeout();
    sec_prof->tcp_cnxn_setup_timeout = spec.tcp_cnxn_setup_timeout();
    sec_prof->tcp_close_timeout = spec.tcp_close_timeout();
    sec_prof->tcp_close_wait_timeout = spec.tcp_close_wait_timeout();

    sec_prof->ip_normalization_en = spec.ip_normalization_en();
    sec_prof->tcp_normalization_en = spec.tcp_normalization_en();
    sec_prof->icmp_normalization_en = spec.icmp_normalization_en();

    sec_prof->ip_ttl_change_detect_en = spec.ip_ttl_change_detect_en();
    sec_prof->ip_rsvd_flags_action = spec.ip_rsvd_flags_action();
    sec_prof->ip_df_action = spec.ip_df_action();
    sec_prof->ip_options_action = spec.ip_options_action();
    sec_prof->ip_invalid_len_action = spec.ip_invalid_len_action();
    sec_prof->ip_normalize_ttl = spec.ip_normalize_ttl();

    sec_prof->icmp_invalid_code_action = spec.icmp_invalid_code_action();
    sec_prof->icmp_deprecated_msgs_drop = spec.icmp_deprecated_msgs_drop();
    sec_prof->icmp_redirect_msg_drop = spec.icmp_redirect_msg_drop();

    sec_prof->tcp_non_syn_first_pkt_drop = spec.tcp_non_syn_first_pkt_drop();
    sec_prof->tcp_syncookie_en = spec.tcp_syncookie_en();
    sec_prof->tcp_split_handshake_detect_en =
        spec.tcp_split_handshake_detect_en();
    sec_prof->tcp_split_handshake_drop = spec.tcp_split_handshake_drop();
    sec_prof->tcp_rsvd_flags_action = spec.tcp_rsvd_flags_action();
    sec_prof->tcp_unexpected_mss_action = spec.tcp_unexpected_mss_action();
    sec_prof->tcp_unexpected_win_scale_action = spec.tcp_unexpected_win_scale_action();
    sec_prof->tcp_urg_ptr_not_set_action = spec.tcp_urg_ptr_not_set_action();
    sec_prof->tcp_urg_flag_not_set_action = spec.tcp_urg_flag_not_set_action();
    sec_prof->tcp_urg_payload_missing_action = spec.tcp_urg_payload_missing_action();
    sec_prof->tcp_rst_with_data_action = spec.tcp_rst_with_data_action();
    sec_prof->tcp_data_len_gt_mss_action = spec.tcp_data_len_gt_mss_action();
    sec_prof->tcp_data_len_gt_win_size_action = spec.tcp_data_len_gt_win_size_action();
    sec_prof->tcp_unexpected_ts_option_action = spec.tcp_unexpected_ts_option_action();
    sec_prof->tcp_unexpected_echo_ts_action = spec.tcp_unexpected_echo_ts_action();
    sec_prof->tcp_ts_not_present_drop = spec.tcp_ts_not_present_drop();
    sec_prof->tcp_invalid_flags_drop = spec.tcp_invalid_flags_drop();
    sec_prof->tcp_nonsyn_noack_drop = spec.tcp_nonsyn_noack_drop();

    return;
}

// create an instance of security profile
hal_ret_t
security_profile_create (nwsec::SecurityProfileSpec& spec,
                         nwsec::SecurityProfileResponse *rsp)
{
    hal_ret_t                      ret;
    nwsec_profile_t                *sec_prof = NULL;
    pd::pd_nwsec_profile_args_t    pd_nwsec_profile_args;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("{}: Creating nwsec profile, id {}", __FUNCTION__,
                    spec.key_or_handle().profile_id());

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_NWSEC_PROFILE_ID_INVALID);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // key-handle field set, but create requires key, not handle
    if (spec.key_or_handle().key_or_handle_case() !=
            SecurityProfileKeyHandle::kProfileId) {
        rsp->set_api_status(types::API_STATUS_NWSEC_PROFILE_ID_INVALID);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // allocate a profile object from slab
    sec_prof = nwsec_profile_alloc_init();
    if (sec_prof == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
        goto end;
    }

    // consume the config
    nwsec_profile_init_from_spec(sec_prof, spec);
    sec_prof->hal_handle = hal_alloc_handle();
    if (sec_prof->hal_handle == HAL_HANDLE_INVALID) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
        goto end;
    }

    // allocate PD resources and finish programming, if any
    pd::pd_nwsec_profile_args_init(&pd_nwsec_profile_args);
    pd_nwsec_profile_args.nwsec_profile = sec_prof;
    ret = pd::pd_nwsec_profile_create(&pd_nwsec_profile_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD security profile create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

    // add this security profile to profile db
    ret = add_nwsec_profile_to_db(sec_prof);
    HAL_ASSERT(ret == HAL_RET_OK);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_profile_status()->set_profile_handle(sec_prof->hal_handle);

end:

    if ((ret != HAL_RET_OK) && (sec_prof != NULL)) {
        HAL_TRACE_DEBUG("Failed to create nwsec profile, id {} ret{}",
                        spec.key_or_handle().profile_id(), ret);
        nwsec_profile_free(sec_prof);
    }
    HAL_TRACE_DEBUG("--------------------- API End ------------------------");
    return ret;
}

// update a security profile instance
hal_ret_t
security_profile_update (nwsec::SecurityProfileSpec& spec,
                         nwsec::SecurityProfileResponse *rsp)
{
    hal_ret_t                      ret;
    nwsec_profile_t                *sec_prof;
    nwsec_profile_t                local_sec_prof;
    pd::pd_nwsec_profile_args_t    pd_nwsec_profile_args;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-Nwsec:{}: Nwsec Update for id {} handle{}",
                    __FUNCTION__,
                    spec.key_or_handle().profile_id(),
                    spec.key_or_handle().profile_handle());

    // key or handle field must be set
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_NWSEC_PROFILE_ID_INVALID);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // lookup this security profile
    sec_prof = nwsec_profile_lookup(spec.key_or_handle());
    if (!sec_prof) {
        rsp->set_api_status(types::API_STATUS_NWSEC_PROFILE_NOT_FOUND);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // Update the incoming config in the local copy
    // Send a local copy down to PD. If PD update succeeds
    // then update the store PI object, else just return
    // error to higher layer. This is to ensure that the
    // PI/PD state doesnt get out of sync upon any PD failures
    nwsec_profile_init_from_spec(&local_sec_prof, spec);
    local_sec_prof.pd = sec_prof->pd;
    
    // Calling PD update
    pd::pd_nwsec_profile_args_init(&pd_nwsec_profile_args);
    pd_nwsec_profile_args.nwsec_profile = &local_sec_prof;
    ret = pd::pd_nwsec_profile_update(&pd_nwsec_profile_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD security profile update failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    } else {
        // Success: Update the store PI object 
        nwsec_profile_init_from_spec(sec_prof, spec);
    }

end:

    HAL_TRACE_DEBUG("PI-Nwsec:{}: Nwsec Update for id {} handle {} ret{}", __FUNCTION__, 
                    spec.key_or_handle().profile_id(), spec.key_or_handle().profile_handle(), ret);
    HAL_TRACE_DEBUG("--------------------- API End ------------------------");
    return ret;
}

hal_ret_t
security_profile_get (nwsec::SecurityProfileGetRequest& req,
                      nwsec::SecurityProfileGetResponse *rsp)
{
    nwsec::SecurityProfileSpec    *spec;
    nwsec_profile_t               *sec_prof;

    // key or handle field must be set
    if (!req.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_NWSEC_PROFILE_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // lookup this security profile
    sec_prof = nwsec_profile_lookup(req.key_or_handle());
    if (!sec_prof) {
        rsp->set_api_status(types::API_STATUS_NWSEC_PROFILE_NOT_FOUND);
        return HAL_RET_INVALID_ARG;
    }

    // fill in the config spec of this profile
    spec = rsp->mutable_spec();
    spec->mutable_key_or_handle()->set_profile_id(sec_prof->profile_id);
    spec->set_cnxn_tracking_en(sec_prof->cnxn_tracking_en);
    spec->set_ipsg_en(sec_prof->ipsg_en);
    spec->set_tcp_rtt_estimate_en(sec_prof->tcp_rtt_estimate_en);
    spec->set_session_idle_timeout(sec_prof->session_idle_timeout);
    spec->set_tcp_cnxn_setup_timeout(sec_prof->tcp_cnxn_setup_timeout);
    spec->set_tcp_close_timeout(sec_prof->tcp_close_timeout);
    spec->set_tcp_close_wait_timeout(sec_prof->tcp_close_wait_timeout);

    spec->set_ip_normalization_en(sec_prof->ip_normalization_en);
    spec->set_tcp_normalization_en(sec_prof->tcp_normalization_en);
    spec->set_icmp_normalization_en(sec_prof->icmp_normalization_en);

    spec->set_ip_ttl_change_detect_en(sec_prof->ip_ttl_change_detect_en);
    spec->set_ip_rsvd_flags_action(
             static_cast<nwsec::NormalizationAction>(sec_prof->ip_rsvd_flags_action));
    spec->set_ip_df_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->ip_df_action));
    spec->set_ip_options_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->ip_options_action));
    spec->set_ip_invalid_len_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->ip_invalid_len_action));
    spec->set_ip_normalize_ttl(sec_prof->ip_normalize_ttl);

    spec->set_icmp_invalid_code_action(
          static_cast<nwsec::NormalizationAction>(sec_prof->icmp_invalid_code_action));
    spec->set_icmp_deprecated_msgs_drop(sec_prof->icmp_deprecated_msgs_drop);
    spec->set_icmp_redirect_msg_drop(sec_prof->icmp_redirect_msg_drop);

    spec->set_tcp_non_syn_first_pkt_drop(sec_prof->tcp_non_syn_first_pkt_drop);
    spec->set_tcp_syncookie_en(sec_prof->tcp_syncookie_en);
    spec->set_tcp_split_handshake_detect_en(sec_prof->tcp_split_handshake_detect_en);
    spec->set_tcp_split_handshake_drop(sec_prof->tcp_split_handshake_drop);
    spec->set_tcp_rsvd_flags_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_rsvd_flags_action));
    spec->set_tcp_unexpected_mss_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_mss_action));
    spec->set_tcp_unexpected_win_scale_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_win_scale_action));
    spec->set_tcp_urg_ptr_not_set_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_urg_ptr_not_set_action));
    spec->set_tcp_urg_flag_not_set_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_urg_flag_not_set_action));
    spec->set_tcp_urg_payload_missing_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_urg_payload_missing_action));
    spec->set_tcp_rst_with_data_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_rst_with_data_action));
    spec->set_tcp_data_len_gt_mss_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_data_len_gt_mss_action));
    spec->set_tcp_data_len_gt_win_size_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_data_len_gt_win_size_action));
    spec->set_tcp_unexpected_ts_option_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_ts_option_action));
    spec->set_tcp_unexpected_echo_ts_action(
              static_cast<nwsec::NormalizationAction>(sec_prof->tcp_unexpected_echo_ts_action));
    spec->set_tcp_ts_not_present_drop(sec_prof->tcp_ts_not_present_drop);
    spec->set_tcp_invalid_flags_drop(sec_prof->tcp_invalid_flags_drop);
    spec->set_tcp_nonsyn_noack_drop(sec_prof->tcp_nonsyn_noack_drop);

    // fill operational state of this profile
    rsp->mutable_status()->set_profile_handle(sec_prof->hal_handle);

    // fill stats, if any, of this profile

    return HAL_RET_OK;
}

}    // namespace hal
