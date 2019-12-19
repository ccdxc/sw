meta:
    id: SECURITY_PROFILES_ACTIVE

profiles:
    - id: SEC_PROF_ACTIVE
      fields:

    - id: SEC_PROF_DEFAULT
      fields:
    
    - id: SEC_PROF_NOOP
      fields:
         cnxn_tracking_en: 0
         ipsg_en: 0
         tcp_rtt_estimate_en: 0
         session_idle_timeout: 0
         tcp_cnxn_setup_timeout: 0
         tcp_close_timeout: 0
         tcp_half_closed_timeout: 0
         ip_normalization_en: 0
         tcp_normalization_en: 0
         icmp_normalization_en: 0
         ip_ttl_change_detect_en: 0
         ip_rsvd_flags_action: NONE 
         ip_df_action: NONE
         ip_options_action: NONE
         ip_invalid_len_action: NONE
         ip_normalize_ttl: 0
         icmp_invalid_code_action: NONE
         icmp_deprecated_msgs_drop: 0
         icmp_redirect_msg_drop: 0
         tcp_non_syn_first_pkt_drop: 0
         tcp_split_handshake_drop: 0
         tcp_rsvd_flags_action: NONE
         tcp_unexpected_mss_action: NONE
         tcp_unexpected_win_scale_action: NONE
         tcp_unexpected_sack_perm_action: NONE
         tcp_urg_ptr_not_set_action: NONE
         tcp_urg_flag_not_set_action: NONE
         tcp_urg_payload_missing_action: NONE
         tcp_rst_with_data_action: NONE
         tcp_data_len_gt_mss_action: NONE
         tcp_data_len_gt_win_size_action: NONE
         tcp_unexpected_ts_option_action: NONE
         tcp_unexpected_sack_option_action: NONE
         tcp_unexpected_echo_ts_action: NONE
         tcp_ts_not_present_drop: 0
         tcp_invalid_flags_drop: 0
         tcp_nonsyn_noack_drop: 0
         tcp_normalize_mss: 0 
         flow_learn_en: 0
         policy_enforce_en: 0

    - id: SEC_PROF_NOOP_FTE
      fields:
         cnxn_tracking_en: 0
         ipsg_en: 0
         tcp_rtt_estimate_en: 0
         session_idle_timeout: 0
         tcp_cnxn_setup_timeout: 0
         tcp_close_timeout: 0
         tcp_half_closed_timeout: 0
         ip_normalization_en: 0
         tcp_normalization_en: 0
         icmp_normalization_en: 0
         ip_ttl_change_detect_en: 0
         ip_rsvd_flags_action: NONE 
         ip_df_action: NONE
         ip_options_action: NONE
         ip_invalid_len_action: NONE
         ip_normalize_ttl: 0
         icmp_invalid_code_action: NONE
         icmp_deprecated_msgs_drop: 0
         icmp_redirect_msg_drop: 0
         tcp_non_syn_first_pkt_drop: 0
         tcp_split_handshake_drop: 0
         tcp_rsvd_flags_action: NONE
         tcp_unexpected_mss_action: NONE
         tcp_unexpected_win_scale_action: NONE
         tcp_unexpected_sack_perm_action: NONE
         tcp_urg_ptr_not_set_action: NONE
         tcp_urg_flag_not_set_action: NONE
         tcp_urg_payload_missing_action: NONE
         tcp_rst_with_data_action: NONE
         tcp_data_len_gt_mss_action: NONE
         tcp_data_len_gt_win_size_action: NONE
         tcp_unexpected_ts_option_action: NONE
         tcp_unexpected_sack_option_action: NONE
         tcp_unexpected_echo_ts_action: NONE
         tcp_ts_not_present_drop: 0
         tcp_invalid_flags_drop: 0
         tcp_nonsyn_noack_drop: 0
         tcp_normalize_mss: 0 
         flow_learn_en: 0
         policy_enforce_en: 1
