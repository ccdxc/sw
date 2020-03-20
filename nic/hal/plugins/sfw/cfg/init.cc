// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nwsec.hpp"
#include "gen/hal/svc/nwsec_svc_gen.hpp"
#include "gen/hal/svc/session_svc_gen.hpp"
#include "gen/proto/nwsec.pb.h"

#define HAL_NWSEC_MAX_RULES 65535

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using nwsec::SecurityProfileSpec;
using nwsec::SecurityProfileResponse;
using nwsec::NormalizationAction;
using nwsec::SecurityProfileStatus;

sdk::lib::indexer *g_rule_stats_indexer;

namespace hal {
namespace plugins {
namespace sfw {

NwSecurityServiceImpl    g_nwsec_svc;

void
sfw_populate_default_enforce_security_profile(SecurityProfileSpec &prof)
{
    prof.mutable_key_or_handle()->set_profile_id(L4_PROFILE_ENFORCE_DEFAULT);
    prof.set_session_idle_timeout(90);
    prof.set_tcp_cnxn_setup_timeout(30);
    prof.set_tcp_close_timeout(15);
    prof.set_tcp_half_closed_timeout(120);
    prof.set_ip_normalize_ttl(0);
    prof.set_tcp_drop_timeout(90);
    prof.set_udp_drop_timeout(60);
    prof.set_icmp_drop_timeout(60);
    prof.set_drop_timeout(60);
    prof.set_tcp_timeout(3600);
    prof.set_udp_timeout(30);
    prof.set_icmp_timeout(6);
    prof.set_cnxn_tracking_en(true);
    prof.set_ipsg_en(false);
    prof.set_tcp_rtt_estimate_en(false);
    prof.set_ip_normalization_en(true);
    prof.set_tcp_normalization_en(true);
    prof.set_icmp_normalization_en(true);
    prof.set_ip_reassembly_en(false);
    prof.set_ip_ttl_change_detect_en(false);
    prof.set_ip_src_guard_en(false);
    prof.set_tcp_rsvd_flags_action(NormalizationAction::NORM_ACTION_DROP);
    prof.set_ip_df_action(NormalizationAction::NORM_ACTION_ALLOW);
    prof.set_ip_options_action(NormalizationAction::NORM_ACTION_ALLOW);
    /*
     * Naples is receiving an ERSPAN packet on uplink with len in ip header 
     * less than the parsed headers. For P4 len in ip header is source of truth.
     * P4 will not be able to trim it less than the parsed headers. For now
     * disabling trimming
     */
    prof.set_ip_invalid_len_action(NormalizationAction::NORM_ACTION_ALLOW);
    prof.set_ip_spoof_pkt_drop(false);
    prof.set_ip_loose_src_routing_pkt_drop(false);
    prof.set_ip_malformed_option_pkt_drop(false);
    prof.set_ip_record_route_option_pkt_drop(false);
    prof.set_ip_strict_src_routing_pkt_drop(false);
    prof.set_ip_ts_option_pkt_drop(false);
    prof.set_ip_unknown_option_pkt_drop(false);
    prof.set_ip_stream_id_option_pkt_drop(false);
    prof.set_ip_rsvd_fld_set_pkt_drop(false);
    prof.set_ip_clear_df_bit(false);
    prof.set_ipv6_anycast_src_drop(false);
    prof.set_ipv6_v4_compatible_addr_drop(false);
    prof.set_ipv6_needless_ip_frag_hdr_drop(false);
    prof.set_ipv6_invalid_options_pkt_drop(false);
    prof.set_ipv6_rsvd_fld_set_pkt_drop(false);
    prof.set_ipv6_rtg_hdr_pkt_drop(false);
    prof.set_ipv6_dst_options_hdr_pkt_drop(false);
    prof.set_ipv6_hop_by_hop_options_pkt_drop(false);
    prof.set_icmp_invalid_code_action(NormalizationAction::NORM_ACTION_ALLOW);
    prof.set_icmp_deprecated_msgs_drop(true);
    prof.set_icmp_redirect_msg_drop(true);
    prof.set_icmp_dst_unreach_ignore_payload(false);
    prof.set_icmp_param_prblm_ignore_payload(false);
    prof.set_icmp_pkt_too_big_ignore_payload(false);
    prof.set_icmp_redirect_ignore_payload(false);
    prof.set_icmp_time_exceed_ignore_payload(false);
    prof.set_icmp_error_drop(false);
    prof.set_icmp_fragments_drop(false);
    prof.set_icmp_large_pkt_drop(false);
    prof.set_icmp_ping_zero_id_drop(false);
    prof.set_icmp_need_frag_suppress(false);
    prof.set_icmp_time_exceed_suppress(false);
    prof.set_icmpv6_large_msg_mtu_small_drop(false);
    prof.set_tcp_split_handshake_drop(false);
    prof.set_tcp_rsvd_flags_action(NormalizationAction::NORM_ACTION_DROP);
    prof.set_tcp_unexpected_mss_action(NormalizationAction::NORM_ACTION_ALLOW);
    prof.set_tcp_unexpected_win_scale_action(NormalizationAction::NORM_ACTION_ALLOW);
    prof.set_tcp_unexpected_sack_perm_action(NormalizationAction::NORM_ACTION_ALLOW);
    prof.set_tcp_urg_ptr_not_set_action(NormalizationAction::NORM_ACTION_ALLOW);
    prof.set_tcp_urg_flag_not_set_action(NormalizationAction::NORM_ACTION_ALLOW);
    prof.set_tcp_urg_payload_missing_action(NormalizationAction::NORM_ACTION_ALLOW);
    prof.set_tcp_rst_with_data_action(NormalizationAction::NORM_ACTION_NONE);
    prof.set_tcp_data_len_gt_mss_action(NormalizationAction::NORM_ACTION_ALLOW);
    prof.set_tcp_data_len_gt_win_size_action(NormalizationAction::NORM_ACTION_ALLOW);
    prof.set_tcp_unexpected_ts_option_action(NormalizationAction::NORM_ACTION_ALLOW);
    prof.set_tcp_unexpected_echo_ts_action(NormalizationAction::NORM_ACTION_DROP);
    prof.set_tcp_unexpected_sack_option_action(NormalizationAction::NORM_ACTION_DROP);
    prof.set_tcp_ts_not_present_drop(true);
    prof.set_tcp_non_syn_first_pkt_drop(true);
    prof.set_tcp_invalid_flags_drop(true);
    prof.set_tcp_nonsyn_noack_drop(true);
    prof.set_tcp_syn_with_data_drop(false);
    prof.set_tcp_syn_ack_with_data_drop(false);
    prof.set_tcp_overlapping_segments_drop(false);
    prof.set_tcp_strip_timestamp_option(false);
    prof.set_tcp_conn_track_bypass_window_err(false);
    prof.set_tcp_conn_track_fin_rst_disable(false);
    prof.set_tcp_urg_flag_ptr_clear(false);
    prof.set_tcp_normalize_mss(0);
    prof.set_multicast_src_drop(true); 
    prof.set_tcp_half_open_session_limit(0);
    prof.set_udp_active_session_limit(0);
    prof.set_icmp_active_session_limit(0);
    prof.set_other_active_session_limit(0);
    prof.set_policy_enforce_en(false);

}

hal_ret_t
sfw_update_default_security_profile(uint32_t id, bool policy_enforce_en)
{
    hal_ret_t                ret;
    SecurityProfileSpec      prof; 
    SecurityProfileResponse  rsp;

    prof.mutable_key_or_handle()->set_profile_id(id);
    prof.set_policy_enforce_en(policy_enforce_en);

    hal::hal_cfg_db_open(CFG_OP_WRITE);
    ret = securityprofile_update(prof, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Could not update default security profile err: {}", ret);
        goto end;
    }

end:
    hal::hal_cfg_db_close();
    return ret;
}

static void
sfw_init_default_mgmt_security_profile (hal_cfg_t *hal_cfg)
{
    hal_ret_t                ret;
    SecurityProfileSpec      prof; 
    SecurityProfileResponse  rsp;

    prof.mutable_key_or_handle()->set_profile_id(L4_PROFILE_MGMT_DEFAULT);

    hal::hal_cfg_db_open(CFG_OP_WRITE);
    ret = securityprofile_create(prof, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Could not create default mgmt security profile err: {}", 
                      ret);
        return;
    }
    hal::hal_cfg_db_close();
}

static void 
sfw_init_default_enforce_security_profile (hal_cfg_t *hal_cfg)
{
    hal_ret_t                ret;
    SecurityProfileSpec      prof; 
    SecurityProfileResponse  rsp;

    sfw_populate_default_enforce_security_profile(prof);

    hal::hal_cfg_db_open(CFG_OP_WRITE);
    ret = securityprofile_create(prof, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Could not create default security profile err: {}", ret);
        return;
    }
    hal::hal_cfg_db_close();
   
    g_hal_state->oper_db()->set_default_security_profile(
                                     rsp.profile_status().profile_handle());
}

static void 
sfw_init_default_host_security_profile(hal_cfg_t *hal_cfg)
{
    hal_ret_t                ret;
    SecurityProfileSpec      prof; 
    SecurityProfileResponse  rsp;

    prof.mutable_key_or_handle()->set_profile_id(L4_PROFILE_HOST_DEFAULT);

    hal::hal_cfg_db_open(CFG_OP_WRITE);
    ret = securityprofile_create(prof, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Could not create default security profile err: {}", ret);
        return;
    }
    hal::hal_cfg_db_close();
   
}

void
svc_reg (ServerBuilder *server_builder, hal::hal_feature_set_t feature_set)
{
    if (!server_builder) {
        return;
    }

    HAL_TRACE_DEBUG("Registering gRPC nwsec services ...");
    if (feature_set == hal::HAL_FEATURE_SET_IRIS) {
        server_builder->RegisterService(&g_nwsec_svc);
    } else if (feature_set == hal::HAL_FEATURE_SET_GFT) {
        // Revisit. DOL was not able to create Tenant with security profile.
        server_builder->RegisterService(&g_nwsec_svc);
    }
    HAL_TRACE_DEBUG("gRPC nwsec services registered ...");
    return;
}

// initialization routine for network module
hal_ret_t
sfwcfg_init (hal_cfg_t *hal_cfg)
{
    // Rule Stats Indexer init
    g_rule_stats_indexer = sdk::lib::indexer::factory(HAL_NWSEC_MAX_RULES, true);
    SDK_ASSERT(g_rule_stats_indexer != NULL);

    svc_reg((ServerBuilder *)hal_cfg->server_builder, hal_cfg->features);
    sfw_init_default_mgmt_security_profile(hal_cfg);
    sfw_init_default_enforce_security_profile(hal_cfg);
    sfw_init_default_host_security_profile(hal_cfg);
    return HAL_RET_OK;
}

// cleanup routine for firewall module
void
sfwcfg_exit (void)
{
}

}    // namespace sfw
}    // namespace plugins
}    // namespace hal
