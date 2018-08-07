meta:
    id: SECURITY_PROFILES_DEFAULT

profiles:
    - id: SEC_PROF_ACTIVE
      fields:
         ip_normalization_en: 0
         tcp_normalization_en: 0
         icmp_normalization_en: 0

    - id: SEC_PROF_DEFAULT
      fields:

    - id: SEC_PROF_IP_RSVD_FLAGS_ACTION_ALLOW
      fields:
         ip_rsvd_flags_action: ALLOW

    - id: SEC_PROF_IP_RSVD_FLAGS_ACTION_DROP
      fields:
         ip_rsvd_flags_action: DROP

    - id: SEC_PROF_IP_RSVD_FLAGS_ACTION_EDIT
      fields:
         ip_rsvd_flags_action: EDIT

    - id: SEC_PROF_IP_DF_ACTION_ALLOW
      fields:
         ip_df_action: ALLOW

    - id: SEC_PROF_IP_DF_ACTION_DROP
      fields:
         ip_df_action: DROP

    - id: SEC_PROF_IP_DF_ACTION_EDIT
      fields:
         ip_df_action: EDIT

    - id: SEC_PROF_IP_OPTIONS_ACTION_ALLOW
      fields:
         ip_options_action: ALLOW

    - id: SEC_PROF_IP_OPTIONS_ACTION_DROP
      fields:
         ip_options_action: DROP

    - id: SEC_PROF_IP_OPTIONS_ACTION_EDIT
      fields:
         ip_options_action: EDIT

    - id: SEC_PROF_IPV6_OPTIONS_ACTION_ALLOW
      fields:
         ip_options_action: ALLOW

    - id: SEC_PROF_IPV6_OPTIONS_ACTION_DROP
      fields:
         ip_options_action: DROP

    - id: SEC_PROF_IPV6_OPTIONS_ACTION_EDIT
      fields:
         ip_options_action: EDIT

    - id: SEC_PROF_IP_INVALID_LEN_ACTION_ALLOW
      fields:
         ip_invalid_len_action: ALLOW

    - id: SEC_PROF_IP_INVALID_LEN_ACTION_DROP
      fields:
         ip_invalid_len_action: DROP

    - id: SEC_PROF_IP_INVALID_LEN_ACTION_EDIT
      fields:
         ip_invalid_len_action: EDIT

    - id: SEC_PROF_IPV6_INVALID_LEN_ACTION_ALLOW
      fields:
         ip_invalid_len_action: ALLOW

    - id: SEC_PROF_IPV6_INVALID_LEN_ACTION_DROP
      fields:
         ip_invalid_len_action: DROP

    - id: SEC_PROF_IPV6_INVALID_LEN_ACTION_EDIT
      fields:
         ip_invalid_len_action: EDIT

    - id: SEC_PROF_ICMP_INVALID_CODE_ECHO_REQ_ACTION_ALLOW
      fields:
         icmp_invalid_code_action: ALLOW

    - id: SEC_PROF_ICMP_INVALID_CODE_ECHO_REQ_ACTION_DROP
      fields:
         icmp_invalid_code_action: DROP

    - id: SEC_PROF_ICMP_INVALID_CODE_ECHO_REQ_ACTION_EDIT
      fields:
         icmp_invalid_code_action: EDIT

    - id: SEC_PROF_ICMP_INVALID_CODE_ECHO_REP_ACTION_ALLOW
      fields:
         icmp_invalid_code_action: ALLOW

    - id: SEC_PROF_ICMP_INVALID_CODE_ECHO_REP_ACTION_DROP
      fields:
         icmp_invalid_code_action: DROP

    - id: SEC_PROF_ICMP_INVALID_CODE_ECHO_REP_ACTION_EDIT
      fields:
         icmp_invalid_code_action: EDIT

    - id: SEC_PROF_ICMP_DEPRECATED_MSGS_TYPE4_DROP_ENABLE
      fields:
         icmp_deprecated_msgs_drop: 1

    - id: SEC_PROF_ICMP_DEPRECATED_MSGS_TYPE4_DROP_DISABLE
      fields:
         icmp_deprecated_msgs_drop: 0

    - id: SEC_PROF_ICMP_DEPRECATED_MSGS_TYPE6_DROP_ENABLE
      fields:
         icmp_deprecated_msgs_drop: 1

    - id: SEC_PROF_ICMP_DEPRECATED_MSGS_TYPE6_DROP_DISABLE
      fields:
         icmp_deprecated_msgs_drop: 0

    - id: SEC_PROF_ICMP_DEPRECATED_MSGS_TYPE15_DROP_ENABLE
      fields:
         icmp_deprecated_msgs_drop: 1

    - id: SEC_PROF_ICMP_DEPRECATED_MSGS_TYPE15_DROP_DISABLE
      fields:
         icmp_deprecated_msgs_drop: 0

    - id: SEC_PROF_ICMP_DEPRECATED_MSGS_TYPE39_DROP_ENABLE
      fields:
         icmp_deprecated_msgs_drop: 1

    - id: SEC_PROF_ICMP_DEPRECATED_MSGS_TYPE39_DROP_DISABLE
      fields:
         icmp_deprecated_msgs_drop: 0

    - id: SEC_PROF_ICMP_DEPRECATED_MSGS_TYPE27_DROP_ENABLE
      fields:
         icmp_deprecated_msgs_drop: 1

    - id: SEC_PROF_ICMP_DEPRECATED_MSGS_TYPE27_DROP_DISABLE
      fields:
         icmp_deprecated_msgs_drop: 0

    - id: SEC_PROF_ICMP_REDIRECT_MSG_DROP_ENABLE
      fields:
         icmp_redirect_msg_drop: 1

    - id: SEC_PROF_ICMP_REDIRECT_MSG_DROP_DISABLE
      fields:
         icmp_redirect_msg_drop: 0

    - id: SEC_PROF_TCP_NON_SYN_FIRST_PKT_DROP_ENABLE
      fields:
         tcp_non_syn_first_pkt_drop: 1

    - id: SEC_PROF_TCP_NON_SYN_FIRST_PKT_DROP_DISABLE
      fields:
         tcp_non_syn_first_pkt_drop: 0

    - id: SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_ENABLE
      fields:
         tcp_split_handshake_drop: 1

    - id: SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_DISABLE
      fields:
         tcp_split_handshake_drop: 0

    - id: SEC_PROF_TCP_RSVD_FLAGS_ACTION_ALLOW
      fields:
         tcp_rsvd_flags_action: ALLOW

    - id: SEC_PROF_TCP_RSVD_FLAGS_ACTION_DROP
      fields:
         tcp_rsvd_flags_action: DROP

    - id: SEC_PROF_TCP_RSVD_FLAGS_ACTION_EDIT
      fields:
         tcp_rsvd_flags_action: EDIT

    - id: SEC_PROF_TCP_UNEXPECTED_MSS_ACTION_ALLOW
      fields:
         tcp_unexpected_mss_action: ALLOW

    - id: SEC_PROF_TCP_UNEXPECTED_MSS_ACTION_DROP
      fields:
         tcp_unexpected_mss_action: DROP

    - id: SEC_PROF_TCP_UNEXPECTED_MSS_ACTION_EDIT
      fields:
         tcp_unexpected_mss_action: EDIT

    - id: SEC_PROF_TCP_UNEXPECTED_WIN_SCALE_ACTION_ALLOW
      fields:
         tcp_unexpected_win_scale_action: ALLOW

    - id: SEC_PROF_TCP_UNEXPECTED_WIN_SCALE_ACTION_DROP
      fields:
         tcp_unexpected_win_scale_action: DROP

    - id: SEC_PROF_TCP_UNEXPECTED_WIN_SCALE_ACTION_EDIT
      fields:
         tcp_unexpected_win_scale_action: EDIT

    - id: SEC_PROF_TCP_UNEXPECTED_SACK_PERM_ACTION_ALLOW
      fields:
         tcp_unexpected_sack_perm_action: ALLOW

    - id: SEC_PROF_TCP_UNEXPECTED_SACK_PERM_ACTION_DROP
      fields:
         tcp_unexpected_sack_perm_action: DROP

    - id: SEC_PROF_TCP_UNEXPECTED_SACK_PERM_ACTION_EDIT
      fields:
         tcp_unexpected_sack_perm_action: EDIT

    - id: SEC_PROF_TCP_URG_PTR_NOT_SET_ACTION_ALLOW
      fields:
         tcp_urg_ptr_not_set_action: ALLOW

    - id: SEC_PROF_TCP_URG_PTR_NOT_SET_ACTION_DROP
      fields:
         tcp_urg_ptr_not_set_action: DROP

    - id: SEC_PROF_TCP_URG_PTR_NOT_SET_ACTION_EDIT
      fields:
         tcp_urg_ptr_not_set_action: EDIT

    - id: SEC_PROF_TCP_URG_FLAG_NOT_SET_ACTION_ALLOW
      fields:
         tcp_urg_flag_not_set_action: ALLOW

    - id: SEC_PROF_TCP_URG_FLAG_NOT_SET_ACTION_DROP
      fields:
         tcp_urg_flag_not_set_action: DROP

    - id: SEC_PROF_TCP_URG_FLAG_NOT_SET_ACTION_EDIT
      fields:
         tcp_urg_flag_not_set_action: EDIT

    - id: SEC_PROF_TCP_URG_PAYLOAD_MISSING_ACTION_ALLOW
      fields:
         tcp_urg_payload_missing_action: ALLOW

    - id: SEC_PROF_TCP_URG_PAYLOAD_MISSING_ACTION_DROP
      fields:
         tcp_urg_payload_missing_action: DROP

    - id: SEC_PROF_TCP_URG_PAYLOAD_MISSING_ACTION_EDIT
      fields:
         tcp_urg_payload_missing_action: EDIT

    - id: SEC_PROF_TCP_RST_WITH_DATA_ACTION_ALLOW
      fields:
         tcp_rst_with_data_action: ALLOW

    - id: SEC_PROF_TCP_RST_WITH_DATA_ACTION_DROP
      fields:
         tcp_rst_with_data_action: DROP

    - id: SEC_PROF_TCP_RST_WITH_DATA_ACTION_EDIT
      fields:
         tcp_rst_with_data_action: EDIT

    - id: SEC_PROF_TCP_DATA_LEN_GT_MSS_ACTION_ALLOW
      fields:
         tcp_data_len_gt_mss_action: ALLOW

    - id: SEC_PROF_TCP_DATA_LEN_GT_MSS_ACTION_DROP
      fields:
         tcp_data_len_gt_mss_action: DROP

    - id: SEC_PROF_TCP_DATA_LEN_GT_MSS_ACTION_EDIT
      fields:
         tcp_data_len_gt_mss_action: EDIT

    - id: SEC_PROF_TCP_DATA_LEN_GT_WIN_SIZE_ACTION_ALLOW
      fields:
         tcp_data_len_gt_win_size_action: ALLOW

    - id: SEC_PROF_TCP_DATA_LEN_GT_WIN_SIZE_ACTION_DROP
      fields:
         tcp_data_len_gt_win_size_action: DROP

    - id: SEC_PROF_TCP_DATA_LEN_GT_WIN_SIZE_ACTION_EDIT
      fields:
         tcp_data_len_gt_win_size_action: EDIT

    - id: SEC_PROF_TCP_UNEXPECTED_TS_OPTION_NEG_ACTION_ALLOW
      fields:
         tcp_unexpected_ts_option_action: ALLOW

    - id: SEC_PROF_TCP_UNEXPECTED_TS_OPTION_NEG_ACTION_DROP
      fields:
         tcp_unexpected_ts_option_action: DROP

    - id: SEC_PROF_TCP_UNEXPECTED_TS_OPTION_NEG_ACTION_EDIT
      fields:
         tcp_unexpected_ts_option_action: EDIT

    - id: SEC_PROF_TCP_UNEXPECTED_TS_OPTION_NOT_NEG_ACTION_ALLOW
      fields:
         tcp_unexpected_ts_option_action: ALLOW

    - id: SEC_PROF_TCP_UNEXPECTED_TS_OPTION_NOT_NEG_ACTION_DROP
      fields:
         tcp_unexpected_ts_option_action: DROP

    - id: SEC_PROF_TCP_UNEXPECTED_TS_OPTION_NOT_NEG_ACTION_EDIT
      fields:
         tcp_unexpected_ts_option_action: EDIT

    - id: SEC_PROF_TCP_UNEXPECTED_SACK_OPTION_NEG_ACTION_ALLOW
      fields:
         tcp_unexpected_sack_option_action: ALLOW

    - id: SEC_PROF_TCP_UNEXPECTED_SACK_OPTION_NEG_ACTION_DROP
      fields:
         tcp_unexpected_sack_option_action: DROP

    - id: SEC_PROF_TCP_UNEXPECTED_SACK_OPTION_NEG_ACTION_EDIT
      fields:
         tcp_unexpected_sack_option_action: EDIT

    - id: SEC_PROF_TCP_UNEXPECTED_SACK_OPTION_NOT_NEG_ACTION_ALLOW
      fields:
         tcp_unexpected_sack_option_action: ALLOW

    - id: SEC_PROF_TCP_UNEXPECTED_SACK_OPTION_NOT_NEG_ACTION_DROP
      fields:
         tcp_unexpected_sack_option_action: DROP

    - id: SEC_PROF_TCP_UNEXPECTED_SACK_OPTION_NOT_NEG_ACTION_EDIT
      fields:
         tcp_unexpected_sack_option_action: EDIT

    - id: SEC_PROF_TCP_UNEXPECTED_ECHO_TS_ACTION_ALLOW
      fields:
         tcp_unexpected_echo_ts_action: ALLOW

    - id: SEC_PROF_TCP_UNEXPECTED_ECHO_TS_ACTION_DROP
      fields:
         tcp_unexpected_echo_ts_action: DROP

    - id: SEC_PROF_TCP_UNEXPECTED_ECHO_TS_ACTION_EDIT
      fields:
         tcp_unexpected_echo_ts_action: EDIT

    - id: SEC_PROF_TCP_TS_NOT_PRESENT_DROP_ENABLE
      fields:
         tcp_ts_not_present_drop: 1

    - id: SEC_PROF_TCP_TS_NOT_PRESENT_DROP_DISABLE
      fields:
         tcp_ts_not_present_drop: 0

    - id: SEC_PROF_TCP_INVALID_FLAGS_1_DROP_ENABLE
      fields:
         tcp_invalid_flags_drop: 1

    - id: SEC_PROF_TCP_INVALID_FLAGS_2_DROP_ENABLE
      fields:
         tcp_invalid_flags_drop: 1

    - id: SEC_PROF_TCP_INVALID_FLAGS_3_DROP_ENABLE
      fields:
         tcp_invalid_flags_drop: 1

    - id: SEC_PROF_TCP_INVALID_FLAGS_4_DROP_ENABLE
      fields:
         tcp_invalid_flags_drop: 1

    - id: SEC_PROF_TCP_INVALID_FLAGS_1_DROP_DISABLE
      fields:
         tcp_invalid_flags_drop: 0

    - id: SEC_PROF_TCP_INVALID_FLAGS_2_DROP_DISABLE
      fields:
         tcp_invalid_flags_drop: 0

    - id: SEC_PROF_TCP_INVALID_FLAGS_3_DROP_DISABLE
      fields:
         tcp_invalid_flags_drop: 0

    - id: SEC_PROF_TCP_INVALID_FLAGS_4_DROP_DISABLE
      fields:
         tcp_invalid_flags_drop: 0

    - id: SEC_PROF_TCP_FLAGS_NONSYN_NOACK_DROP_ENABLE
      fields:
         tcp_nonsyn_noack_drop: 1

    - id: SEC_PROF_TCP_FLAGS_NONSYN_NOACK_DROP_DISABLE
      fields:
         tcp_nonsyn_noack_drop: 0

    - id: SEC_PROF_TCP_UNEXPECTED_TS_OPTION_EDIT_SACK_1
      fields:
         tcp_unexpected_ts_option_action: EDIT

    - id: SEC_PROF_TCP_UNEXPECTED_TS_OPTION_EDIT_SACK_2
      fields:
         tcp_unexpected_ts_option_action: EDIT

    - id: SEC_PROF_TCP_UNEXPECTED_TS_OPTION_EDIT_SACK_3
      fields:
         tcp_unexpected_ts_option_action: EDIT

    - id: SEC_PROF_TCP_UNEXPECTED_TS_OPTION_EDIT_WS_SACK_1
      fields:
         tcp_unexpected_ts_option_action: EDIT

    - id: SEC_PROF_TCP_UNEXPECTED_WIN_SCALE_ACTION_EDIT_SACK_4
      fields:
         tcp_unexpected_win_scale_action: EDIT

    - id: SEC_PROF_TCP_UNEXPECTED_WIN_SCALE_ACTION_EDIT_MSS_SACK_PERM
      fields:
         tcp_unexpected_win_scale_action: EDIT

    - id: SEC_PROF_TCP_UNEXPECTED_MSS_ACTION_EDIT_WIN_SCALE_SACK_PERM
      fields:
         tcp_unexpected_mss_action: EDIT

    - id: SEC_PROF_TCP_UNEXPECTED_MSS_ACTION_EDIT_WIN_SCALE_ONLY
      fields:
         tcp_unexpected_mss_action: EDIT

    - id: SEC_PROF_IP_TTL_NORMALIZE_32
      fields:
         ip_normalize_ttl: 32

    - id: SEC_PROF_IPV6_HOP_LIMIT_NORMALIZE_32
      fields:
         ip_normalize_ttl: 32

    - id: SEC_PROF_TCP_NORMALIZE_MSS_WITH_MSS_OPTION
      fields:
         tcp_normalize_mss: 1460

    - id: SEC_PROF_TCP_NORMALIZE_MSS_WITHOUT_MSS_OPTION
      fields:
         tcp_normalize_mss: 1460

    - id: SEC_PROF_TCP_NORMALIZE_MSS_WITH_WS_ONLY_OPTION
      fields:
         tcp_normalize_mss: 1460

    - id: SEC_PROF_FIREWALL
      fields:
        cnxn_tracking_en: True


