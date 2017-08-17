# Python file to generate the spec files for normalization
'''
NormalizationAction {
  NORM_ACTION_NONE    = 0;
  NORM_ACTION_ALLOW   = 1;
  NORM_ACTION_DROP    = 2;
  NORM_ACTION_EDIT    = 3;
}
'''

knobs_list = [
#    ('bool', 'cnxn_tracking_en', '3'),
#    ('bool', 'ipsg_en', '4'),
#    ('bool', 'tcp_rtt_estimate_en', '5'),
#    ('uint32', 'session_idle_timeout', '6'),
#    ('uint32', 'tcp_cnxn_setup_timeout', '7'),
#    ('uint32', 'tcp_close_timeout', '8'),
#    ('uint32', 'tcp_close_wait_timeout', '9'),
####
#    ('bool', 'ip_normalization_en', '10'),
#    ('bool', 'tcp_normalization_en', '11'),
#    ('bool', 'icmp_normalization_en', '12'),
# IP normalization knobs
    # TBD: TTL change detect
    #('bool', 'ip_ttl_change_detect_en', '20'),
    ('NormalizationAction', 'ip_rsvd_flags_action', '21'),
    ('NormalizationAction', 'ip_df_action', '22'),
    ('NormalizationAction', 'ip_options_action', '23'),
    ('NormalizationAction', 'ip_invalid_len_action', '24'),
    ('uint32', 'ip_normalize_ttl', '25'),
# ICMP/ICMPv6 normalization knobs
    ('NormalizationAction', 'icmp_invalid_code_action', '30'),
    ('bool', 'icmp_deprecated_msgs_drop', '31'),
    ('bool', 'icmp_redirect_msg_drop', '32'),
# TCP normalization knobs
    ('bool', 'tcp_non_syn_first_pkt_drop', '40'),
    ('bool', 'tcp_syncookie_en', '41'),
    ('bool', 'tcp_split_handshake_detect_en', '42'),
    ('bool', 'tcp_split_handshake_drop', '43'),
    ('NormalizationAction', 'tcp_rsvd_flags_action', '44'),
    ('NormalizationAction', 'tcp_unexpected_mss_action', '45'),
    ('NormalizationAction', 'tcp_unexpected_win_scale_action', '46'),
    ('NormalizationAction', 'tcp_urg_ptr_not_set_action', '47'),
    ('NormalizationAction', 'tcp_urg_flag_not_set_action', '48'),
    ('NormalizationAction', 'tcp_urg_payload_missing_action', '49'),
    ('NormalizationAction', 'tcp_rst_with_data_action', '50'),
    ('NormalizationAction', 'tcp_data_len_gt_mss_action', '51'),
    ('NormalizationAction', 'tcp_data_len_gt_win_size_action', '52'),
    ('NormalizationAction', 'tcp_unexpected_ts_option_action', '53'),
    ('NormalizationAction', 'tcp_unexpected_echo_ts_action', '54'),
    ('bool', 'tcp_ts_not_present_drop', '55'),
    ('bool', 'tcp_invalid_flags_drop', '56'),
    ('bool', 'tcp_flags_nonsyn_noack_drop', '57')]

print 'meta:'
print '    id: SECURITY_PROFILES'
print
print 'profiles:'
print '    - id: SEC_PROF_ACTIVE'
print '      fields:'
print
print '    - id: SEC_PROF_DEFAULT'
print '      fields:'
print

drop_id_offset = 100
edit_id_offset = 200

for knob in knobs_list:
    (type, knob, num) = knob
    if type == 'bool':
        print '    - id: SEC_PROF_%s_ENABLE' % (knob.upper())
        print '      fields:'
        print '         %s: 1' % (knob)
        print
        print '    - id: SEC_PROF_%s_DISABLE' % (knob.upper())
        print '      fields:'
        print '         %s: 0' % (knob)
        print
    elif type == 'NormalizationAction':
        print '    - id: SEC_PROF_%s_ALLOW' % (knob.upper())
        print '      fields:'
        print '         %s: ALLOW' % (knob)
        print
        print '    - id: SEC_PROF_%s_DROP' % (knob.upper())
        print '      fields:'
        print '         %s: DROP' % (knob)
        print
        print '    - id: SEC_PROF_%s_EDIT' % (knob.upper())
        print '      fields:'
        print '         %s: EDIT' % (knob)
        print


