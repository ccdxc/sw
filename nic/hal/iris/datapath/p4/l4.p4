// {C} Copyright 2017- Pensando Systems Inc. All rights reserved.


/*****************************************************************************/
/* Metadata for L4 features                                                  */
/*****************************************************************************/
header_type l4_metadata_t {
    fields {
        profile_idx                              : 8;

        // TRUE if connection tracking is ON.
        flow_conn_track                          : 1;

        // Knobs will be classified based on Layer/Protocol to which they are applied
        // Like IP, ICMP, TCP, UDP....
        // Further they are classified based on 1 bit knobs vs 2 bit knobs.
        // 1 bit knobs:
        //    enable/disable, allow/drop
        // 2 bit knobs:
        //    allow/drop/clear/trim/
        //      edit - Can be modifying or trimming the packet

        // High level Normalizaiton knobs.
        icmp_normalization_en                    : 1;
        tcp_normalization_en                     : 1;
        ip_normalization_en                      : 1;

        // IP Normalization
        // ip_options_action: Clear all IP options seen. This is for the inner packet in case of tunneled..
        // ip_invalid_len_action: Check for Outer IP only. Packet can be trimmed based on config.
        // ip_normalize_ttl: If non-zero, replace all IP packet ttl with this value.
        ip_rsvd_flags_action                     : 2;

        // IP Independent
        ip_fragment_drop                         : 1;

        ip_df_action                             : 2;
        ip_options_action                        : 2;
        ip_invalid_len_action                    : 2;
        ip_normalize_ttl                         : 8;

        // ICMP Normalization
        icmp_deprecated_msgs_drop                : 1;
        icmp_redirect_msg_drop                   : 1;
        icmp_invalid_code_action                 : 2;

        // TCP - Normalization
        // tcp_unexpected_mss_action: We can Remove the MSS option
        // tcp_unexpected_win_scale_action: We can Remove the MSS option
        // tcp_unexpected_sack_perm_action: We can Remove the SACK_PERM option
        // tcp_unexpected_echo_ts_action: Packet with no ACK flag but echo_ts is non-zero
        // tcp_unexpected_ts_option_action: timestamp option present, but not negotiated and this is not a SYN packet
        // tcp_unexpected_sack_option_action: sack option present, but not negotiated
        tcp_rsvd_flags_action                    : 2;
        tcp_unexpected_mss_action                : 2;
        tcp_unexpected_win_scale_action          : 2;
        tcp_unexpected_sack_perm_action          : 2;
        tcp_urg_ptr_not_set_action               : 2;
        tcp_urg_flag_not_set_action              : 2;
        tcp_urg_payload_missing_action           : 2;
        tcp_unexpected_echo_ts_action            : 2;
        tcp_rst_with_data_action                 : 2;
        tcp_data_len_gt_mss_action               : 2;
        tcp_data_len_gt_win_size_action          : 2;
        tcp_unexpected_ts_option_action          : 2;
        tcp_unexpected_sack_option_action        : 2;
        tcp_ts_not_present_drop                  : 1;
        tcp_flags_nonsyn_noack_drop              : 1;
        tcp_invalid_flags_drop                   : 1;

        // TCP - Independent
        tcp_non_syn_first_pkt_drop               : 1;
        tcp_split_handshake_detect_en            : 1;
        tcp_split_handshake_drop                 : 1;
        tcp_normalize_mss                        : 16;

        // Flow Knobs:
        // ip_ttl_change_detect_en: TRUE if IP TTL change detection is on and mark the exception seen bits.
        //                          We would need exceptions bits in flow too in addition to session state.
        ip_ttl_change_detect_en                  : 1;

        // Configuration Knobs:
        flow_learn_cfg_en                        : 1;  // Enable Flow Learning
        policy_enf_cfg_en                        : 1;  // Enable Policy Enforcement
        ip_bm_mc_policy_enf_cfg_en               : 1;  // Enable Policy Encforcement for MC & BC. 
                                                       //   Valid only if policy_enf_cfg_en is enabled
        uuc_fl_pe_sup_en                         : 1;


        // Other metadata
        // tcp_data_len: TCP Packet data length excluding options
        tcp_data_len                             : 16;
        tcp_split_handshake_detected             : 1;

        // Saving in metadata, in case session state related normalization
        // checks are done outside the session state action handling.
        tcp_rcvr_win_sz                          : 32;
        tcp_ts_option_negotiated                 : 1;
        tcp_sack_perm_option_negotiated          : 1;

        // Once the complete processing is done for packet this field
        // needs to be ORed with the initator or responder exception_seen bits
        // in the session state table data.
        // Will also need to send the exceptions seen for the current packet
        // to CPU (not from session state), which will be useful for ALGs
        exceptions_seen         : 16;           // list of exceptions seen
    }
}

metadata l4_metadata_t l4_metadata;

action l4_profile(icmp_normalization_en,
                  tcp_normalization_en,
                  ip_normalization_en,
                  ip_rsvd_flags_action,
                  ip_df_action,
                  ip_options_action,
                  ip_invalid_len_action,
                  ip_fragment_drop,
                  ip_ttl_change_detect_en,
                  ip_normalize_ttl,
                  icmp_deprecated_msgs_drop,
                  icmp_redirect_msg_drop,
                  icmp_invalid_code_action,
                  tcp_rsvd_flags_action,
                  tcp_unexpected_mss_action,
                  tcp_unexpected_win_scale_action,
                  tcp_unexpected_sack_perm_action,
                  tcp_urg_ptr_not_set_action,
                  tcp_urg_flag_not_set_action,
                  tcp_urg_payload_missing_action,
                  tcp_unexpected_echo_ts_action,
                  tcp_rst_with_data_action,
                  tcp_data_len_gt_mss_action,
                  tcp_data_len_gt_win_size_action,
                  tcp_unexpected_ts_option_action,
                  tcp_unexpected_sack_option_action,
                  tcp_ts_not_present_drop,
                  tcp_flags_nonsyn_noack_drop,
                  tcp_invalid_flags_drop,
                  tcp_non_syn_first_pkt_drop,
                  tcp_split_handshake_detect_en,
                  tcp_split_handshake_drop,
                  tcp_normalize_mss,
                  flow_learn_cfg_en,
                  policy_enf_cfg_en,
                  ip_bm_mc_policy_enf_cfg_en,
                  uuc_fl_pe_sup_en) {

    modify_field(l4_metadata.ip_normalization_en, ip_normalization_en);
    modify_field(l4_metadata.ip_rsvd_flags_action, ip_rsvd_flags_action);
    modify_field(l4_metadata.ip_df_action, ip_df_action);
    modify_field(l4_metadata.ip_options_action, ip_options_action);
    modify_field(l4_metadata.ip_invalid_len_action, ip_invalid_len_action);
    modify_field(l4_metadata.ip_normalize_ttl, ip_normalize_ttl);
    modify_field(l4_metadata.ip_fragment_drop, ip_fragment_drop);
    modify_field(l4_metadata.icmp_deprecated_msgs_drop, icmp_deprecated_msgs_drop);
    modify_field(l4_metadata.icmp_redirect_msg_drop, icmp_redirect_msg_drop);
    modify_field(l4_metadata.icmp_invalid_code_action, icmp_invalid_code_action);
    modify_field(l4_metadata.tcp_rsvd_flags_action, tcp_rsvd_flags_action);
    modify_field(l4_metadata.tcp_unexpected_mss_action, tcp_unexpected_mss_action);
    modify_field(l4_metadata.tcp_unexpected_win_scale_action, tcp_unexpected_win_scale_action);
    modify_field(l4_metadata.tcp_unexpected_sack_perm_action, tcp_unexpected_sack_perm_action);
    modify_field(l4_metadata.tcp_urg_ptr_not_set_action, tcp_urg_ptr_not_set_action);
    modify_field(l4_metadata.tcp_urg_flag_not_set_action, tcp_urg_flag_not_set_action);
    modify_field(l4_metadata.tcp_urg_payload_missing_action, tcp_urg_payload_missing_action);
    modify_field(l4_metadata.tcp_unexpected_echo_ts_action, tcp_unexpected_echo_ts_action);
    modify_field(l4_metadata.tcp_rst_with_data_action, tcp_rst_with_data_action);
    modify_field(l4_metadata.tcp_data_len_gt_mss_action, tcp_data_len_gt_mss_action);
    modify_field(l4_metadata.tcp_data_len_gt_win_size_action, tcp_data_len_gt_win_size_action);
    modify_field(l4_metadata.tcp_unexpected_ts_option_action, tcp_unexpected_ts_option_action);
    modify_field(l4_metadata.tcp_unexpected_sack_option_action, tcp_unexpected_sack_option_action);
    modify_field(l4_metadata.tcp_ts_not_present_drop, tcp_ts_not_present_drop);
    modify_field(l4_metadata.tcp_flags_nonsyn_noack_drop, tcp_flags_nonsyn_noack_drop);
    modify_field(l4_metadata.tcp_invalid_flags_drop, tcp_invalid_flags_drop);
    modify_field(l4_metadata.tcp_non_syn_first_pkt_drop, tcp_non_syn_first_pkt_drop);
    modify_field(l4_metadata.tcp_split_handshake_detect_en, tcp_split_handshake_detect_en);
    modify_field(l4_metadata.tcp_split_handshake_drop, tcp_split_handshake_drop);
    modify_field(l4_metadata.ip_ttl_change_detect_en, ip_ttl_change_detect_en);
    modify_field(l4_metadata.tcp_normalize_mss, tcp_normalize_mss);
    modify_field(l4_metadata.flow_learn_cfg_en, flow_learn_cfg_en);
    modify_field(l4_metadata.policy_enf_cfg_en, policy_enf_cfg_en);
    modify_field(l4_metadata.ip_bm_mc_policy_enf_cfg_en, ip_bm_mc_policy_enf_cfg_en);
    modify_field(l4_metadata.uuc_fl_pe_sup_en, uuc_fl_pe_sup_en);

    if (icmp.valid == TRUE) {
        modify_field(l4_metadata.icmp_normalization_en, icmp_normalization_en);
    }
    if (tcp.valid == TRUE) {
        modify_field(l4_metadata.tcp_normalization_en, tcp_normalization_en);
        // Adjust the TCP Data Len for IPv6 option case. We are adding it
        // here as the stage0 we couldn't bring in the below fields
        //   l3_metadata.ipv6_options_len
        //   l3_metadata.inner_ipv6_options_len
        // into input_mapping_native and input_mapping_tunnel routies due
        // key makers being full.
        if (l3_metadata.ip_option_seen == TRUE) {
            if (inner_ipv6.valid == TRUE and tunnel_metadata.tunnel_terminate == TRUE) {
                subtract(l4_metadata.tcp_data_len, l4_metadata.tcp_data_len, l3_metadata.inner_ipv6_options_len);
            } 
            if (ipv6.valid == TRUE and tunnel_metadata.tunnel_terminate == FALSE) {
                subtract(l4_metadata.tcp_data_len, l4_metadata.tcp_data_len, l3_metadata.ipv6_options_len);
            }
        }
    }

    if (p4plus_to_p4.valid == TRUE) {
        f_p4plus_to_p4_1();
    }
}

@pragma stage 2
table l4_profile {
    reads {
        l4_metadata.profile_idx : exact;
    }
    actions {
        l4_profile;
        nop;
    }
    size : L4_PROFILE_TABLE_SIZE;
}

control process_l4_profile {
    apply(l4_profile);
}

/*****************************************************************************
 * Validate TCP session state
 * 1. session state dependent Normalization
 * 2. TCP Seq Number Validation
 * 3. TCP State Validation
 * 4. RTT Calculation
 *
 * We only need one delta value per session to accommodate TCP syn cookie
 * feature (Phase 2 feature)
 * 1. For adjusted seq number for packets coming from responder we will
 *    subtract the delta from recevied seq number.
 * 2. For adjusted ack number for packets coming from Initiator we will
 *    add the delta to received ack number.
 * Both above operations have to be signed arithmatic but 32 bit wraparound.
******************************************************************************/
action tcp_session_state_info(iflow_tcp_seq_num,
                       iflow_tcp_ack_num,
                       iflow_tcp_win_sz, iflow_tcp_win_scale,
                       iflow_tcp_mss,
                       iflow_tcp_state,
                       rflow_tcp_state,
                       iflow_exceptions_seen,
                       rflow_tcp_seq_num,
                       rflow_tcp_ack_num,
                       rflow_tcp_win_sz, rflow_tcp_win_scale,
                       rflow_tcp_mss,
                       syn_cookie_delta,
                       rflow_exceptions_seen,
                       flow_rtt_seq_check_enabled,
                       iflow_rtt_in_progress,
                       iflow_rtt, iflow_rtt_seq_no, iflow_rtt_timestamp,
                       rflow_rtt_in_progress,
                       rflow_rtt, rflow_rtt_seq_no, rflow_rtt_timestamp,
                       iflow_tcp_ws_option_sent, iflow_tcp_ts_option_sent,
                       iflow_tcp_sack_perm_option_sent, tcp_ts_option_negotiated,
                       tcp_sack_perm_option_negotiated) {

    /* dummy ops to keep compiler happy */
    modify_field(scratch_metadata.iflow_tcp_seq_num, iflow_tcp_seq_num);
    modify_field(scratch_metadata.iflow_tcp_ack_num, iflow_tcp_ack_num);
    modify_field(scratch_metadata.iflow_tcp_win_sz, iflow_tcp_win_sz);
    modify_field(scratch_metadata.iflow_tcp_win_scale, iflow_tcp_win_scale);
    modify_field(scratch_metadata.iflow_tcp_state, iflow_tcp_state);
    modify_field(scratch_metadata.iflow_exceptions_seen, iflow_exceptions_seen);
    modify_field(scratch_metadata.rflow_tcp_seq_num, rflow_tcp_seq_num);
    modify_field(scratch_metadata.rflow_tcp_ack_num, rflow_tcp_ack_num);
    modify_field(scratch_metadata.rflow_tcp_win_sz, rflow_tcp_win_sz);
    modify_field(scratch_metadata.rflow_tcp_win_scale, rflow_tcp_win_scale);
    modify_field(scratch_metadata.rflow_tcp_state, rflow_tcp_state);
    modify_field(scratch_metadata.rflow_exceptions_seen, rflow_exceptions_seen);
    modify_field(scratch_metadata.syn_cookie_delta, syn_cookie_delta);
    modify_field(scratch_metadata.iflow_tcp_ws_option_sent, iflow_tcp_ws_option_sent);
    modify_field(scratch_metadata.iflow_tcp_ts_option_sent, iflow_tcp_ts_option_sent);
    modify_field(scratch_metadata.iflow_tcp_sack_perm_option_sent, iflow_tcp_sack_perm_option_sent);
    modify_field(l4_metadata.tcp_ts_option_negotiated, tcp_ts_option_negotiated);
    modify_field(l4_metadata.tcp_sack_perm_option_negotiated, tcp_sack_perm_option_negotiated);

    // RTT
    modify_field(scratch_metadata.flow_rtt_seq_check_enabled, flow_rtt_seq_check_enabled);
    modify_field(scratch_metadata.flow_rtt_in_progress, iflow_rtt_in_progress);
    modify_field(scratch_metadata.flow_rtt, iflow_rtt);
    modify_field(scratch_metadata.flow_rtt_seq_no, iflow_rtt_seq_no);
    modify_field(scratch_metadata.flow_rtt_timestamp, iflow_rtt_timestamp);
    modify_field(scratch_metadata.flow_rtt_in_progress, rflow_rtt_in_progress);
    modify_field(scratch_metadata.flow_rtt, rflow_rtt);
    modify_field(scratch_metadata.flow_rtt_seq_no, rflow_rtt_seq_no);
    modify_field(scratch_metadata.flow_rtt_timestamp, rflow_rtt_timestamp);

    // To avoid checks of flow_role in the code and to prevent using
    // registers, we will split the code into initator and responder
    // sections. Even though there might be a bit of code duplication
    // it will help in keeping code straight forward and debugging
    // and maintenance easier.
    if (flow_info_metadata.flow_role == TCP_FLOW_INITIATOR) {
        // PACKET FROM INITATOR
        // Commenting the below line to get the correct size for scale and win_sz
        // modify_field(l4_metadata.tcp_rcvr_win_sz, (rflow_tcp_win_sz << rflow_tcp_win_scale));
        modify_field(scratch_metadata.tcp_mss, rflow_tcp_mss);

       if (l4_metadata.tcp_normalization_en == P4_FEATURE_ENABLED) {
           tcp_session_normalization();
       }
       if (syn_cookie_delta != 0) {
           modify_field(scratch_metadata.adjusted_ack_num, (tcp.ackNo + syn_cookie_delta));
       }
       modify_field(scratch_metadata.tcp_seq_num_hi, (tcp.seqNo + l4_metadata.tcp_data_len -1));
       if (iflow_tcp_state == FLOW_STATE_ESTABLISHED and
           rflow_tcp_state == FLOW_STATE_ESTABLISHED and
           (tcp.flags == TCP_FLAG_ACK or
            tcp.flags == (TCP_FLAG_ACK|TCP_FLAG_PSH)) and
           (l4_metadata.tcp_rcvr_win_sz != 0) and
           (((l4_metadata.tcp_data_len != 0) and
             (tcp.seqNo == iflow_tcp_seq_num) and
             (scratch_metadata.tcp_seq_num_hi < (rflow_tcp_ack_num + l4_metadata.tcp_rcvr_win_sz))) or
            ((l4_metadata.tcp_data_len == 0) and
             (rflow_tcp_ack_num -1 <= tcp.seqNo) and
             (tcp.seqNo < rflow_tcp_ack_num + l4_metadata.tcp_rcvr_win_sz)))) {

            if (tcp.ackNo > scratch_metadata.iflow_tcp_ack_num and
                tcp.ackNo <= scratch_metadata.rflow_tcp_seq_num) {
                modify_field(scratch_metadata.iflow_tcp_ack_num, tcp.ackNo);
                modify_field(scratch_metadata.iflow_tcp_win_sz, tcp.window);
            }
            if (scratch_metadata.tcp_seq_num_hi >  iflow_tcp_seq_num) {
                // Only if we are seeing a new high sequence number than we saved already
                modify_field(scratch_metadata.iflow_tcp_seq_num, scratch_metadata.tcp_seq_num_hi + 1);
            }
            // Now Update the PHV Fields to accomodate the adjusted seq and ack number in case
            if (scratch_metadata.syn_cookie_delta != 0) {
                modify_field(tcp.ackNo, scratch_metadata.adjusted_ack_num);
            }
           // Exit the action routine or Jump to RTT Computation if enabled.
        }
        // If SYN or FIN is set in a packet then we can increment the tcp_seq_num_hi by 1 here
        if ((tcp.flags & TCP_FLAG_FIN == TCP_FLAG_FIN) or
            (tcp.flags & TCP_FLAG_SYN == TCP_FLAG_SYN)) {
            add_to_field (scratch_metadata.tcp_seq_num_hi, 1);
        }

        if (l4_metadata.tcp_data_len != 0 and
            l4_metadata.tcp_rcvr_win_sz != 0) {

            if (tcp.seqNo == iflow_tcp_seq_num and
                (scratch_metadata.tcp_seq_num_hi < (rflow_tcp_ack_num + l4_metadata.tcp_rcvr_win_sz))) {
                // goto INITIATOR_TCP_STATE_TRANSITION:
            }
            if ((tcp.seqNo >= rflow_tcp_ack_num) and
                (scratch_metadata.tcp_seq_num_hi < (rflow_tcp_ack_num + l4_metadata.tcp_rcvr_win_sz))) {
                // non-overlapping within the window but not the next one we are
                // expecting, possible re-ordering of segments happens in between
                bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_PACKET_REORDER);
                // goto INITIATOR_TCP_STATE_TRANSITION:
            }
            if (tcp.seqNo < rflow_tcp_ack_num and
                (scratch_metadata.tcp_seq_num_hi < rflow_tcp_ack_num)) {
                // full retransmit of packet we have seen before, still acceptable
                // We shouldn't drop this packet as we might have seen the ack but
                // sender might not have rcvd the ack as it could be dropped after us.
                // We will have to update the ack # and window fromt the packet.
                bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_FULL_REXMIT);
                // Update state (ack and window) and exit.
                // goto INITIATOR_TCP_SESSION_UPDATE:
            }

            if ((tcp.seqNo < rflow_tcp_ack_num) and
                (scratch_metadata.tcp_seq_num_hi < (rflow_tcp_ack_num + l4_metadata.tcp_rcvr_win_sz))) {
                // left side overlap, still acceptable
                bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_PARTIAL_OVERLAP);
                // goto INITIATOR_TCP_STATE_TRANSITION:
            }


           // either segment is overlapping on the right or fully outside the
           // window to the right or engulfing the whole window size and
           // exceeding the window sz - drop
           bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_OUT_OF_WINDOW);
           modify_field(control_metadata.drop_reason, DROP_TCP_OUT_OF_WINDOW);
           drop_packet();
           // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
       }

       // This case is also handled in best packet case but it assumed that no flags
       // other than ACK+PSH were set.
       if ((l4_metadata.tcp_data_len == 0) and (l4_metadata.tcp_rcvr_win_sz != 0)) {
           // Initiator flow can also be installed in INIT state after the flow
           // miss is sent to ARM CPU which will reinject the packet and this time
           // we will hit the flow that is installed. This is to avoid tcp session
           // tracking to see the ARM injected packet as a SYN REXMIT.
           if (iflow_tcp_state == FLOW_STATE_INIT and tcp.flags == TCP_FLAG_SYN) {
               modify_field(scratch_metadata.iflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD);
               // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
           }
           if (iflow_tcp_state == FLOW_STATE_TCP_SYN_RCVD and
               (tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_ACK) == TCP_FLAG_SYN) and
               (tcp.seqNo + 1 == iflow_tcp_seq_num)) {
               // TRACE_DBG("SYN rexmit detected\n");
               bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_SYN_REXMIT);
               //  We can jump to end of the program where exception bit is copied into session state.
               // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
           }
           // Before we do the RFC check, we need to make sure that we received
           // ack from the responder side, then only rflow_tcp_ack_num will be
           // valid
           if (rflow_tcp_state <= FLOW_STATE_TCP_SYN_RCVD) {
                // This will be hit when simultanoes open is done and initiator
                // is responding with an ACK to responder
                // goto INITIATOR_TCP_STATE_TRANSITION:
           }
           if ((rflow_tcp_ack_num -1 <= tcp.seqNo) and
               (tcp.seqNo < rflow_tcp_ack_num + l4_metadata.tcp_rcvr_win_sz)) {
                // goto INITIATOR_TCP_STATE_TRANSITION:
           } else {
               bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_OUT_OF_WINDOW);
               modify_field(control_metadata.drop_reason, DROP_TCP_OUT_OF_WINDOW);
               drop_packet();
               // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
           }
       }
       // if segment lenght is > 0 but receiver window size is 0
       // this is not actually a security breach, may be the sender
       // didn't receive the window advertisement with sz 0 yet, simply
       // retransmit, but increment a counter
       if ((l4_metadata.tcp_data_len != 0) and (l4_metadata.tcp_rcvr_win_sz == 0)) {
           bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_WIN_ZERO_DROP);
           modify_field(control_metadata.drop_reason, DROP_TCP_WIN_ZERO_DROP);
           drop_packet();
           // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
       }

       // Per RFC: if both segment length and receive window are zero, only packets that
       // are allowed are empty ACK or RST or URG pkts
       // How is FIN prevented here ?
       if ((l4_metadata.tcp_data_len == 0) and (l4_metadata.tcp_rcvr_win_sz == 0)) {
           // SYN Packets will not hit this case because in pre established state
           // window size will be non-zero
           if (tcp.seqNo == rflow_tcp_ack_num or
               tcp.seqNo == (rflow_tcp_ack_num - 1)) {
               // goto INITIATOR_TCP_STATE_TRANSITION:
           } else {
               bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_OUT_OF_WINDOW);
               modify_field(control_metadata.drop_reason, DROP_TCP_OUT_OF_WINDOW);
               drop_packet();
               // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
           }
       }

#if 0
       We will handle this case as part of state transiton handling. In the
       case of FLOW_STATE_FIN_RCVD, we will add the check to not allow data
       to the right of already received FIN even if the window permits.
       // This case was handled in best packet case and later, but we checked for
       // 1. Established and (ACK or ACK + PSH) only
       // 2. Established with any Flags
       // But a third case is what if we moved to a FIN_RCVD state because  we
       // saw a reordered FIN, then we have to still allow the data.
#endif /* 0 */

       // Default case is to proceed to TCP State Transitions.
       // All invalid sequence number should have been taken care above this.

// INITIATOR_TCP_STATE_TRANSITION:
       // TCP connection state tracking

      // A reordered frame, retransmit or partial window frome in established
      // state with no SYN, FIN and RST should go through fast processing
      if ((tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_FIN|TCP_FLAG_RST) == 0) and
          scratch_metadata.iflow_tcp_state == FLOW_STATE_ESTABLISHED) {

          // goto INITIATOR_TCP_SESSION_UPDATE
       }
       // RST Handling
       if (tcp.flags & TCP_FLAG_RST == TCP_FLAG_RST) {
           if (scratch_metadata.iflow_tcp_state != FLOW_STATE_RESET) {
               // Goal here is to reset the connection only if the sequence
               // number matches the next expected sequence number from the
               // point of view of the receiver who is receiving the reset.
               // For all other resets with sequence number in window, we 
               // assuming that the challenge ack rfc is supported and we
               // eventually get the reset with right sequence number.
               // RFC 5961
               if (scratch_metadata.rflow_tcp_state < FLOW_STATE_TCP_ACK_RCVD) {
                   modify_field(scratch_metadata.iflow_tcp_state, FLOW_STATE_RESET);
                   modify_field(scratch_metadata.rflow_tcp_state, FLOW_STATE_RESET);
                   // Mark the packet to redirect to CPU.
                   // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
               }
               // If we really want to add a knob to support end hosts which support
               // and which don't support challenge ack then we can add here and
               // handle accordingly
               // RFC 5961
               if (tcp.seqNo == scratch_metadata.rflow_tcp_ack_num) {
                   modify_field(scratch_metadata.iflow_tcp_state, FLOW_STATE_RESET);
                   modify_field(scratch_metadata.rflow_tcp_state, FLOW_STATE_RESET);
                   // Mark the packet to redirect to CPU.
                   // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
               } else {
                   // We will let it go and let the Challege ack take care of it
                   // to send the reset with the right seq number
                   // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
               }
           } else {
               // Duplicate RST, Let it go.
               // Not adding a exception here for now.
           }
           // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
       }
       if (scratch_metadata.iflow_tcp_state >= FLOW_STATE_ESTABLISHED and
           tcp.flags & TCP_FLAG_SYN == TCP_FLAG_SYN) {
           bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_UNEXPECTED_PKT);
           modify_field(control_metadata.drop_reason, DROP_TCP_UNEXPECTED_PKT);
           drop_packet();
          // INITIATOR_TCP_SESSION_STATE_INFO_EXIT:
       }
       // If we receive FIN in any pre-established state,
       if (iflow_tcp_state < FLOW_STATE_ESTABLISHED) {
           if (tcp.flags & TCP_FLAG_FIN == TCP_FLAG_FIN) {
               bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_UNEXPECTED_PKT);
               modify_field(control_metadata.drop_reason, DROP_TCP_UNEXPECTED_PKT);
               drop_packet();
               // INITIATOR_TCP_SESSION_STATE_INFO_EXIT:
           }
       }
       // we will do a switch case based on iflow_tcp_state for reducing the number
       // of instructions executed.
       if (scratch_metadata.iflow_tcp_state == FLOW_STATE_TCP_SYN_RCVD)  {
          // INITIATOR_TCP_SESSION_STATE_INFO_EXIT:
       }

       if (scratch_metadata.iflow_tcp_state == FLOW_STATE_TCP_SYN_RCVD)  {
           if ((scratch_metadata.rflow_tcp_state == FLOW_STATE_TCP_SYN_ACK_RCVD) and
               (tcp.flags & TCP_FLAG_ACK == TCP_FLAG_ACK) and
               (tcp.ackNo == scratch_metadata.rflow_tcp_seq_num)) {
               modify_field(scratch_metadata.iflow_tcp_state, FLOW_STATE_ESTABLISHED);
               modify_field(scratch_metadata.rflow_tcp_state, FLOW_STATE_ESTABLISHED);
               modify_field(scratch_metadata.iflow_tcp_seq_num, scratch_metadata.tcp_seq_num_hi + 1);
               modify_field(scratch_metadata.iflow_tcp_ack_num, tcp.ackNo);
               modify_field(scratch_metadata.iflow_tcp_win_sz, tcp.window);
               modify_field(tcp.ackNo, scratch_metadata.adjusted_ack_num);
               // We could have Data along with this ACK.
               // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
           }
           if ((scratch_metadata.rflow_tcp_state == FLOW_STATE_TCP_SYN_RCVD) and
               (tcp.flags & (TCP_FLAG_ACK) == (TCP_FLAG_ACK)) and
               (tcp.ackNo == scratch_metadata.rflow_tcp_seq_num)) {
               modify_field(scratch_metadata.iflow_tcp_state, FLOW_STATE_TCP_SYN_ACK_RCVD);
               modify_field(scratch_metadata.iflow_tcp_ack_num, tcp.ackNo);
               modify_field(scratch_metadata.iflow_tcp_win_sz, tcp.window);
               modify_field(tcp.ackNo, scratch_metadata.adjusted_ack_num);
               // goto INITIATOR_TCP_SESSION_UPDATE
           }
           // I don't think we need to handle FIN here right ?
           // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
       }
       if (scratch_metadata.iflow_tcp_state == FLOW_STATE_TCP_SYN_ACK_RCVD)  {
           // Do we have to handle FIN in this case ?
           // If initator is in this state then no packet from initiator
           // will the state to established, It should be a ACK packet 
           // from responder which can move to established state which is
           // is handled later in responder section.
           // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
       }
       if (scratch_metadata.iflow_tcp_state == FLOW_STATE_TCP_ACK_RCVD)  {
           // Not a valid state for initiator
           // Should we drop the packet and have a default exception.
          // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
       }
       if (scratch_metadata.iflow_tcp_state == FLOW_STATE_ESTABLISHED)  {

           if (tcp.flags & TCP_FLAG_FIN == TCP_FLAG_FIN and
               scratch_metadata.rflow_tcp_state != FLOW_STATE_FIN_RCVD) {

               modify_field (scratch_metadata.iflow_tcp_state, FLOW_STATE_FIN_RCVD);
               // Redirect to ARM CPU for FTE to start aging this flow.
               modify_field(capri_intrinsic.tm_replicate_en, TRUE);
               modify_field(capri_intrinsic.tm_replicate_ptr, P4_NW_MCAST_INDEX_FIN_COPY);

               // goto INITIATOR_TCP_SESSION_UPDATE
           }
           if (tcp.flags & TCP_FLAG_FIN == TCP_FLAG_FIN and
               scratch_metadata.rflow_tcp_state == FLOW_STATE_FIN_RCVD) {

               modify_field (scratch_metadata.iflow_tcp_state, FLOW_STATE_BIDIR_FIN_RCVD);
               modify_field (scratch_metadata.rflow_tcp_state, FLOW_STATE_BIDIR_FIN_RCVD);
               // Redirect to ARM CPU for FTE to start aging this flow.
               modify_field(capri_intrinsic.tm_replicate_en, TRUE);
               modify_field(capri_intrinsic.tm_replicate_ptr, P4_NW_MCAST_INDEX_FIN_COPY);
               // goto INITIATOR_TCP_SESSION_UPDATE
           }
       }
       if (scratch_metadata.iflow_tcp_state == FLOW_STATE_FIN_RCVD or
           scratch_metadata.iflow_tcp_state == FLOW_STATE_BIDIR_FIN_RCVD) {
           // We should drop any data beyond the FIN Sequence number whiich
           // we should have cached as part of iflow_tcp_seq_num
           // Below check cannot be >= as all the subsequent packets after FIN
           // which can be ACK only packets will have a sequence number of
           // FIN sequence + 1 which is what we saved in rflow_tcp_seq_num
           if (scratch_metadata.tcp_seq_num_hi >= scratch_metadata.iflow_tcp_seq_num) {
               bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_DATA_AFTER_FIN);
               modify_field(control_metadata.drop_reason, DROP_TCP_DATA_AFTER_FIN);
               drop_packet();
               // INITIATOR_TCP_SESSION_STATE_INFO_EXIT:
           }
           // goto INITIATOR_TCP_SESSION_UPDATE
       }
       if (scratch_metadata.iflow_tcp_state == FLOW_STATE_RESET) {
           // No point letting any packets go through other than RESET itself in
           // case the earlier one is lost. This is handled as part of RST FLAG
           // Handling.

           // Here we need to handle any other packets that are coming
           // when we are reset state. we can just drop them
           bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_NON_RST_PKT_AFTER_RST);
           modify_field(control_metadata.drop_reason, DROP_TCP_NON_RST_PKT_AFTER_RST);
           drop_packet();
          // INITIATOR_TCP_SESSION_STATE_INFO_EXIT:
       }


// INITIATOR_TCP_SESSION_UPDATE:
        // Making sure that some random ack number is not updated as this
        // can completely throw us off
        if (tcp.ackNo > scratch_metadata.iflow_tcp_ack_num and
            tcp.ackNo <= scratch_metadata.rflow_tcp_seq_num) {
            modify_field(scratch_metadata.iflow_tcp_ack_num, tcp.ackNo);
            modify_field(scratch_metadata.iflow_tcp_win_sz, tcp.window);
        }
        if (scratch_metadata.tcp_seq_num_hi >= iflow_tcp_seq_num) {
            // Only if we are seeing a new high sequence number than we saved already
            modify_field(scratch_metadata.iflow_tcp_seq_num, scratch_metadata.tcp_seq_num_hi + 1);
        }
        // Now Update the PHV Fields to accomodate the adjusted seq and ack number in case
        if (scratch_metadata.syn_cookie_delta != 0) {
            modify_field(tcp.ackNo, scratch_metadata.adjusted_ack_num);
        }


// INITIATOR_TCP_SESSION_STATE_INFO_EXIT:
        bit_or(scratch_metadata.iflow_exceptions_seen, scratch_metadata.iflow_exceptions_seen, l4_metadata.exceptions_seen);
        // Exit

// This will be implemented as a seperate function and will only be called for
// data flowing in Established connections only and for the best packet cases only.
// INITIATOR_TCP_RTT_CALCULATION:

    } else {
        // PACKET FROM RESPONDER
        // Commenting the below line to get the correct size for scale and win_sz
        // modify_field(l4_metadata.tcp_rcvr_win_sz, (iflow_tcp_win_sz << iflow_tcp_win_scale));
        modify_field(scratch_metadata.tcp_mss, iflow_tcp_mss);

       if (l4_metadata.tcp_normalization_en == P4_FEATURE_ENABLED) {
           tcp_session_normalization();
       }
       if (syn_cookie_delta != 0) {
           modify_field(scratch_metadata.adjusted_seq_num, (tcp.seqNo - syn_cookie_delta));
       }
       modify_field(scratch_metadata.tcp_seq_num_hi, (scratch_metadata.adjusted_seq_num + l4_metadata.tcp_data_len - 1));
       if (iflow_tcp_state == FLOW_STATE_ESTABLISHED and
           rflow_tcp_state == FLOW_STATE_ESTABLISHED and
           (tcp.flags == TCP_FLAG_ACK or
            tcp.flags == (TCP_FLAG_ACK|TCP_FLAG_PSH)) and
           (l4_metadata.tcp_rcvr_win_sz != 0) and
           (((l4_metadata.tcp_data_len != 0) and
             (scratch_metadata.adjusted_seq_num == scratch_metadata.rflow_tcp_seq_num) and
             (scratch_metadata.tcp_seq_num_hi < (scratch_metadata.iflow_tcp_ack_num + l4_metadata.tcp_rcvr_win_sz))) or
            ((l4_metadata.tcp_data_len == 0) and
             (scratch_metadata.iflow_tcp_ack_num -1 <= scratch_metadata.adjusted_seq_num) and
             (scratch_metadata.adjusted_seq_num < scratch_metadata.iflow_tcp_ack_num + l4_metadata.tcp_rcvr_win_sz)))) {

            if (tcp.ackNo > scratch_metadata.rflow_tcp_ack_num and
                tcp.ackNo <= scratch_metadata.iflow_tcp_seq_num) {
                modify_field(scratch_metadata.iflow_tcp_ack_num, tcp.ackNo);
                modify_field(scratch_metadata.iflow_tcp_win_sz, tcp.window);
            }
            if (scratch_metadata.tcp_seq_num_hi >  scratch_metadata.rflow_tcp_seq_num) {
                // Only if we are seeing a new high sequence number than we saved already
                modify_field(scratch_metadata.rflow_tcp_seq_num, scratch_metadata.tcp_seq_num_hi + 1);
            }
            // Now Update the PHV Fields to accomodate the adjusted seq and ack number in case
            if (scratch_metadata.syn_cookie_delta != 0) {
                modify_field(tcp.seqNo, scratch_metadata.adjusted_seq_num);
            }
        }
        // Exit the action routine or Jump to RTT Computation if enabled.

        // If SYN or FIN is set in a packet then we can increment the tcp_seq_num_hi by 1 here
        // This will make sure that all seq number checks take into account and do correct
        // boudary checks like out of window.
        if ((tcp.flags & TCP_FLAG_FIN == TCP_FLAG_FIN) or
            (tcp.flags & TCP_FLAG_SYN == TCP_FLAG_SYN)) {
            add_to_field (scratch_metadata.tcp_seq_num_hi, 1);
        }

        if (l4_metadata.tcp_data_len != 0 and
            l4_metadata.tcp_rcvr_win_sz != 0) {

            if (scratch_metadata.adjusted_seq_num == rflow_tcp_seq_num and
                (scratch_metadata.tcp_seq_num_hi < (iflow_tcp_ack_num + l4_metadata.tcp_rcvr_win_sz))) {
                // goto RESPONDER_TCP_STATE_TRANSITION:
            }
            if (scratch_metadata.adjusted_seq_num < iflow_tcp_ack_num and
                (scratch_metadata.tcp_seq_num_hi < iflow_tcp_ack_num)) {
                // full retransmit of packet we have seen before, still acceptable
                // We shouldn't drop this packet as we might have seen the ack but
                // sender might not have rcvd the ack as it could be dropped after us.
                // We will have to update the ack # and window fromt the packet.
                bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_FULL_REXMIT);
                // Update state (ack and window) and exit.
                // goto RESPONDER_TCP_SESSION_UPDATE:
            }

            if ((scratch_metadata.adjusted_seq_num < iflow_tcp_ack_num) and
                (scratch_metadata.tcp_seq_num_hi < (iflow_tcp_ack_num + l4_metadata.tcp_rcvr_win_sz))) {
                // left side overlap, still acceptable
                bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_PARTIAL_OVERLAP);
                // goto RESPONDER_TCP_STATE_TRANSITION:
            }

            if ((scratch_metadata.adjusted_seq_num >= iflow_tcp_ack_num) and
                (scratch_metadata.tcp_seq_num_hi < (iflow_tcp_ack_num + l4_metadata.tcp_rcvr_win_sz))) {
                // non-overlapping within the window but not the next one we are
                // expecting, possible re-ordering of segments happens in between
                bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_PACKET_REORDER);
                // goto RESPONDER_TCP_STATE_TRANSITION:
            }

           // either segment is overlapping on the right or fully outside the
           // window to the right or engulfing the whole window size and
           // exceeding the window sz - drop
           bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_OUT_OF_WINDOW);
           modify_field(control_metadata.drop_reason, DROP_TCP_OUT_OF_WINDOW);
           drop_packet();
           // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
       }

       // This case is also handled in best packet case but it assumed that no flags
       // other than ACK+PSH were set.
       if ((l4_metadata.tcp_data_len == 0) and (l4_metadata.tcp_rcvr_win_sz != 0)) {
           // How do we let the first packet coming from Responder go to TCP State Transiton logic.
           // Only valid value at this point in the session is initiator TCP Seq Number.
           if (rflow_tcp_state < FLOW_STATE_ESTABLISHED and iflow_tcp_state < FLOW_STATE_ESTABLISHED) {
               if ((tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_ACK) == TCP_FLAG_SYN|TCP_FLAG_ACK) and
                   (tcp.ackNo == iflow_tcp_seq_num)) {
                   // This is the gneuine SYN_ACK from responder. Most common case when opening a new
                   // new connection
                  // goto RESPONDER_TCP_STATE_TRANSITION:
               }
               // From RFC: 793
               // In all states except SYN-SENT, all reset (RST) segments are validated by
               // checking their SEQ-fields.  A reset is valid if its sequence number is in the window.
               // In the SYN-SENT state (a RST received in response to an initial SYN),
               // the RST is acceptable if the ACK field acknowledges the SYN.
               if ((tcp.flags & (TCP_FLAG_RST) == TCP_FLAG_RST) and
                   (tcp.ackNo != iflow_tcp_seq_num)) {
                   bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_RST_WITH_INVALID_ACK_NUM);
                   modify_field(control_metadata.drop_reason, DROP_TCP_RST_WITH_INVALID_ACK_NUM);
                   drop_packet();
               }
               if ((tcp.flags & (TCP_FLAG_ACK) == TCP_FLAG_ACK) and
                   (tcp.ackNo == iflow_tcp_seq_num)) {
                   // This will be true for any genuine packet that is sent by responder in response to
                   // initiator SYN. This could a packet FIM or RST.
                   // goto RESPONDER_TCP_STATE_TRANSITION:
               }
               if (tcp.flags & (TCP_FLAG_SYN) == TCP_FLAG_SYN) {
                   // If this is a simultaneous open then only SYN flag will be set and there is
                   // nothing much to verify.
                   // goto RESPONDER_TCP_STATE_TRANSITION:
               }
               // For rest of the cases, we will fall through the RFC check for seq numbers
               // and any illegitimate packet will be dropped.
           }
           if ((iflow_tcp_ack_num -1 <= scratch_metadata.adjusted_seq_num) and
               (scratch_metadata.adjusted_seq_num < iflow_tcp_ack_num + l4_metadata.tcp_rcvr_win_sz)) {
                // goto RESPONDER_TCP_STATE_TRANSITION:
           } else {
               bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_OUT_OF_WINDOW);
               modify_field(control_metadata.drop_reason, DROP_TCP_OUT_OF_WINDOW);
               drop_packet();
               // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
           }
       }
       // if segment lenght is > 0 but receiver window size is 0
       // this is not actually a security breach, may be the sender
       // didn't receive the window advertisement with sz 0 yet, simply
       // retransmit, but increment a counter
       if ((l4_metadata.tcp_data_len != 0) and (l4_metadata.tcp_rcvr_win_sz == 0)) {
           bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_WIN_ZERO_DROP);
           modify_field(control_metadata.drop_reason, DROP_TCP_WIN_ZERO_DROP);
           drop_packet();
           // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
       }

       // Per RFC: if both segment length and receive window are zero, only packets that
       // are allowed are empty ACK or RST or URG pkts
       // How is FIN prevented here ?
       if ((l4_metadata.tcp_data_len == 0) and (l4_metadata.tcp_rcvr_win_sz == 0)) {
           // SYN Packets will not hit this case because in pre established state
           // window size will be non-zero
           if (scratch_metadata.adjusted_seq_num == iflow_tcp_ack_num or
               scratch_metadata.adjusted_seq_num == (iflow_tcp_ack_num - 1)) {
               // goto RESPONDER_TCP_STATE_TRANSITION:
           } else {
               bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_OUT_OF_WINDOW);
               modify_field(control_metadata.drop_reason, DROP_TCP_OUT_OF_WINDOW);
               drop_packet();
               // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
           }
       }
       // Default case is to proceed to TCP State Transitions.
       // All invalid sequence number should have been taken care above this.

// RESPONDER_TCP_STATE_TRANSITION:
       // TCP connection state tracking

      // A reordered frame, retransmit or partial window frome in established
      // state with no SYN, FIN and RST should go through fast processing
      if ((tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_FIN|TCP_FLAG_RST) == 0) and
          scratch_metadata.iflow_tcp_state == FLOW_STATE_ESTABLISHED) {

          // goto INITIATOR_TCP_SESSION_UPDATE
       }
       // RST Handling
       if (tcp.flags & TCP_FLAG_RST == TCP_FLAG_RST) {
           if (scratch_metadata.rflow_tcp_state != FLOW_STATE_RESET) {
               // Goal here is to reset the connection only if the sequence
               // number matches the next expected sequence number from the
               // point of view of the receiver who is receiving the reset.
               // For all other resets with sequence number in window, we 
               // assuming that the challenge ack rfc is supported and we
               // eventually get the reset with right sequence number.
               // RFC 5961
               if (scratch_metadata.iflow_tcp_state < FLOW_STATE_TCP_ACK_RCVD) {
                   modify_field(scratch_metadata.iflow_tcp_state, FLOW_STATE_RESET);
                   modify_field(scratch_metadata.rflow_tcp_state, FLOW_STATE_RESET);
                   // Mark the packet to redirect to CPU.
                   // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
               }
               // If we really want to add a knob to support end hosts which support
               // and which don't support challenge ack then we can add here and
               // handle accordingly
               // RFC 5961
               if (tcp.seqNo == scratch_metadata.iflow_tcp_ack_num) {
                   modify_field(scratch_metadata.iflow_tcp_state, FLOW_STATE_RESET);
                   modify_field(scratch_metadata.rflow_tcp_state, FLOW_STATE_RESET);
                   // Mark the packet to redirect to CPU.
                   // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
               } else {
                   // We will let it go and let the Challege ack take care of it
                   // to send the reset with the right seq number
                   // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
               }
           } else {
               // Duplicate RST, Let it go.
               // Not adding a exception here for now.
           }
           // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
       }
       if (scratch_metadata.rflow_tcp_state >= FLOW_STATE_ESTABLISHED and
           tcp.flags & TCP_FLAG_SYN == TCP_FLAG_SYN) {
           bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_UNEXPECTED_PKT);
           modify_field(control_metadata.drop_reason, DROP_TCP_UNEXPECTED_PKT);
           drop_packet();
          // RESPONDER_TCP_SESSION_STATE_INFO_EXIT:
       }
       // If we receive FIN in any pre-established state,
       if (rflow_tcp_state < FLOW_STATE_ESTABLISHED) {
           if (tcp.flags & TCP_FLAG_FIN == TCP_FLAG_FIN) {
               bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_UNEXPECTED_PKT);
               modify_field(control_metadata.drop_reason, DROP_TCP_UNEXPECTED_PKT);
               drop_packet();
              // RESPONDER_TCP_SESSION_STATE_INFO_EXIT:
           }
       }
       // we will do a switch case based on rflow_tcp_state for reducing the number
       // of instructions executed.
        if (rflow_tcp_state == FLOW_STATE_INIT) {
            // Initator flow has to be in SYN_RCVD state.
            if (tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_ACK) == (TCP_FLAG_SYN|TCP_FLAG_ACK) and
                tcp.ackNo == iflow_tcp_seq_num) {
                modify_field(scratch_metadata.rflow_tcp_state, FLOW_STATE_TCP_SYN_ACK_RCVD);
                if (scratch_metadata.iflow_tcp_ws_option_sent == TRUE and
                    tcp_option_ws.valid == TRUE) {
                    modify_field (scratch_metadata.rflow_tcp_win_scale, tcp_option_ws.value);
                } else {
                    modify_field (scratch_metadata.iflow_tcp_win_scale, 0);
                }
                if (scratch_metadata.iflow_tcp_ts_option_sent == TRUE  and
                    tcp_option_timestamp.valid == TRUE) {
                    modify_field (l4_metadata.tcp_ts_option_negotiated, 1);
                }
                if (scratch_metadata.iflow_tcp_sack_perm_option_sent == TRUE  and
                    tcp_option_sack_perm.valid == TRUE) {
                    modify_field (l4_metadata.tcp_sack_perm_option_negotiated, 1);
                }
                if (tcp_option_mss.valid == TRUE) {
                    modify_field (scratch_metadata.rflow_tcp_mss, tcp_option_mss.value);
                }
                // We will have to update the responder flows seq and ack directly here as the
                // commmon code will check for greater than and it might not do the right stuff
                // as the initial values are zero when the session entry is programmed.
                modify_field(scratch_metadata.rflow_tcp_seq_num, scratch_metadata.tcp_seq_num_hi + 1);
                modify_field(scratch_metadata.rflow_tcp_ack_num, tcp.ackNo);
                modify_field(scratch_metadata.rflow_tcp_win_sz, tcp.window);
                // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
            }
            if (tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_ACK) == (TCP_FLAG_ACK) and
                tcp.ackNo == iflow_tcp_seq_num) {
                bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_SPLIT_HANDSHAKE_DETECTED);
                if (l4_metadata.tcp_split_handshake_drop == NORMALIZATION_ACTION_DROP) {
                    modify_field(control_metadata.drop_reason, DROP_TCP_SPLIT_HANDSHAKE);
                    drop_packet();
                } else {
                    modify_field(scratch_metadata.rflow_tcp_state, FLOW_STATE_TCP_ACK_RCVD);
                    modify_field(scratch_metadata.rflow_tcp_ack_num, tcp.ackNo);
                    modify_field(scratch_metadata.rflow_tcp_win_sz, tcp.window);
                }
                // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
            }
            if (tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_ACK) == (TCP_FLAG_SYN) and
                tcp.ackNo == 0) {
                // This is simultaneous open case, for now we are treating it under the bucket of split handshake.
                bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_SPLIT_HANDSHAKE_DETECTED);
                if (l4_metadata.tcp_split_handshake_drop == NORMALIZATION_ACTION_DROP) {
                    modify_field(control_metadata.drop_reason, DROP_TCP_SPLIT_HANDSHAKE);
                    drop_packet();
                    // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
                } else {
                    modify_field(scratch_metadata.rflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD);
                    if (scratch_metadata.iflow_tcp_ws_option_sent == TRUE and
                        tcp_option_ws.valid == TRUE) {
                        modify_field (scratch_metadata.rflow_tcp_win_scale, tcp_option_ws.value);
                    } else {
                        modify_field (scratch_metadata.iflow_tcp_win_scale, 0);
                    }
                    if (scratch_metadata.iflow_tcp_ts_option_sent == TRUE  and
                        tcp_option_timestamp.valid == TRUE) {
                        modify_field (l4_metadata.tcp_ts_option_negotiated, 1);
                    }
                    if (scratch_metadata.iflow_tcp_sack_perm_option_sent == TRUE  and
                        tcp_option_sack_perm.valid == TRUE) {
                        modify_field (l4_metadata.tcp_sack_perm_option_negotiated, 1);
                    }
                    if (tcp_option_mss.valid == TRUE) {
                        modify_field (scratch_metadata.rflow_tcp_mss, tcp_option_mss.value);
                    }
                    // We will have to update the responder flows seq and ack directly here as the
                    // commmon code will check for greater than and it might not do the right stuff
                    // as the initial values are zero when the session entry is programmed.
                    // Not updating window as ACK bit is not set.
                    modify_field(scratch_metadata.rflow_tcp_seq_num, scratch_metadata.tcp_seq_num_hi + 1);
                    // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
                }
            }
            // Drop any other packet for now.
            bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_INVALID_RESPONDER_FIRST_PKT);
            modify_field(control_metadata.drop_reason, DROP_TCP_INVALID_RESPONDER_FIRST_PKT);
            drop_packet();
        }
       if (scratch_metadata.rflow_tcp_state == FLOW_STATE_TCP_SYN_RCVD)  {
           if ((scratch_metadata.iflow_tcp_state == FLOW_STATE_TCP_SYN_RCVD) and
               (tcp.flags & (TCP_FLAG_ACK) == (TCP_FLAG_ACK)) and
               (tcp.ackNo == scratch_metadata.iflow_tcp_seq_num)) {
               modify_field(scratch_metadata.rflow_tcp_state, FLOW_STATE_TCP_SYN_ACK_RCVD);
               // goto RESPONDER_TCP_SESSION_UPDATE
           }
           if ((scratch_metadata.iflow_tcp_state == FLOW_STATE_TCP_SYN_ACK_RCVD) and
               (tcp.flags & TCP_FLAG_ACK == TCP_FLAG_ACK) and
               (tcp.ackNo == scratch_metadata.iflow_tcp_seq_num)) {
               modify_field(scratch_metadata.iflow_tcp_state, FLOW_STATE_ESTABLISHED);
               modify_field(scratch_metadata.rflow_tcp_state, FLOW_STATE_ESTABLISHED);
               // We could have Data along with this ACK.
               // goto RESPONDER_TCP_SESSION_UPDATE
           }
           // I don't think we need to handle FIN here right ?
           // We are not dropping the packet as there could be SYN Retransmit
           // If we have to strict, we can check for SYN also and drop rest of the
           // packets.
           // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
       }
       if (scratch_metadata.rflow_tcp_state == FLOW_STATE_TCP_ACK_RCVD)  {
           // iflow_tcp_state should be in FLOW_STATE_TCP_SYN_RCVD only
           if (tcp.flags & (TCP_FLAG_SYN) == (TCP_FLAG_SYN)) {

                modify_field(scratch_metadata.rflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD);
                if (scratch_metadata.iflow_tcp_ws_option_sent == TRUE and
                    tcp_option_ws.valid == TRUE) {
                    modify_field (scratch_metadata.rflow_tcp_win_scale, tcp_option_ws.value);
                } else {
                    modify_field (scratch_metadata.iflow_tcp_win_scale, 0);
                }
                if (scratch_metadata.iflow_tcp_ts_option_sent == TRUE  and
                    tcp_option_timestamp.valid == TRUE) {
                    modify_field (l4_metadata.tcp_ts_option_negotiated, 1);
                }
                if (scratch_metadata.iflow_tcp_sack_perm_option_sent == TRUE  and
                    tcp_option_sack_perm.valid == TRUE) {
                    modify_field (l4_metadata.tcp_sack_perm_option_negotiated, 1);
                }
                if (tcp_option_mss.valid == TRUE) {
                    modify_field (scratch_metadata.rflow_tcp_mss, tcp_option_mss.value);
                }
                // We will have to update the responder flows seq and ack directly here as the
                // We will have to update the responder flows seq and ack directly here as the
                // commmon code will check for greater than and it might not do the right stuff
                // as the initial values are zero when the session entry is programmed.
                // Not updating window as ACK bit is not set.
                modify_field(scratch_metadata.rflow_tcp_seq_num, scratch_metadata.tcp_seq_num_hi + 1);
                // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
           }
           // We are not dropping the packet as there could be ACKs Retransmit
           // If we have to strict, we can check for SYN also and drop rest of the
           // packets.
           // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
       }
       if (scratch_metadata.rflow_tcp_state == FLOW_STATE_TCP_SYN_ACK_RCVD)  {
           // No packet received from Responder will help to move out of this state.
           // Do we have to handle FIN in this case ?
           // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
       }

       if (scratch_metadata.rflow_tcp_state == FLOW_STATE_ESTABLISHED)  {
           if (tcp.flags & TCP_FLAG_FIN == TCP_FLAG_FIN and
               scratch_metadata.iflow_tcp_state != FLOW_STATE_FIN_RCVD) {

               modify_field (scratch_metadata.rflow_tcp_state, FLOW_STATE_FIN_RCVD);
               // Redirect to ARM CPU for FTE to start aging this flow.
               modify_field(capri_intrinsic.tm_replicate_en, TRUE);
               modify_field(capri_intrinsic.tm_replicate_ptr, P4_NW_MCAST_INDEX_FIN_COPY);
               // goto RESPONDER_TCP_SESSION_UPDATE
           }
           if (tcp.flags & TCP_FLAG_FIN == TCP_FLAG_FIN and
               scratch_metadata.iflow_tcp_state == FLOW_STATE_FIN_RCVD) {

               modify_field (scratch_metadata.rflow_tcp_state, FLOW_STATE_BIDIR_FIN_RCVD);
               modify_field (scratch_metadata.iflow_tcp_state, FLOW_STATE_BIDIR_FIN_RCVD);
               // Redirect to ARM CPU for FTE to start aging this flow.
               modify_field(capri_intrinsic.tm_replicate_en, TRUE);
               modify_field(capri_intrinsic.tm_replicate_ptr, P4_NW_MCAST_INDEX_FIN_COPY);
               // goto RESPONDER_TCP_SESSION_UPDATE
           }
       }
       if (scratch_metadata.rflow_tcp_state == FLOW_STATE_FIN_RCVD or
           scratch_metadata.rflow_tcp_state == FLOW_STATE_BIDIR_FIN_RCVD) {
           // We should drop any data beyond the FIN Sequence number whiich
           // we should have cached as part of rflow_tcp_seq_num
           // Below check cannot be >= as all the subsequent packets after FIN
           // which can be ACK only packets will have a sequence number of
           // FIN sequence + 1 which is what we saved in rflow_tcp_seq_num
           if (scratch_metadata.tcp_seq_num_hi >= scratch_metadata.rflow_tcp_seq_num) {
               bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_DATA_AFTER_FIN);
               modify_field(control_metadata.drop_reason, DROP_TCP_DATA_AFTER_FIN);
               drop_packet();
               // RESPONDER_TCP_SESSION_STATE_INFO_EXIT:
           }
           // goto RESPONDER_TCP_SESSION_UPDATE
       }
       if (scratch_metadata.rflow_tcp_state == FLOW_STATE_RESET) {
           // No point letting any packets go through other than RESET itself in
           // case the earlier one is lost. This is handled as part of RST FLAG
           // Handling.

           // Here we need to handle any other packets that are coming
           // when we are reset state. we can just drop them
           bit_or(l4_metadata.exceptions_seen, l4_metadata.exceptions_seen, TCP_NON_RST_PKT_AFTER_RST);
           modify_field(control_metadata.drop_reason, DROP_TCP_NON_RST_PKT_AFTER_RST);
           drop_packet();
          // RESPONDER_TCP_SESSION_STATE_INFO_EXIT:
       }

// RESPONDER_TCP_SESSION_UPDATE:
        // Making sure that some random ack number is not updated as this
        // can completely throw us off
        if (tcp.ackNo > scratch_metadata.rflow_tcp_ack_num and
            tcp.ackNo <= scratch_metadata.iflow_tcp_seq_num) {
            modify_field(scratch_metadata.rflow_tcp_ack_num, tcp.ackNo);
            modify_field(scratch_metadata.rflow_tcp_win_sz, tcp.window);
        }
        if (scratch_metadata.tcp_seq_num_hi >=  scratch_metadata.rflow_tcp_seq_num) {
            // Only if we are seeing a new high sequence number than we saved already
            modify_field(scratch_metadata.rflow_tcp_seq_num, scratch_metadata.tcp_seq_num_hi + 1);
        }
        // Now Update the PHV Fields to accomodate the adjusted seq and ack number in case
        if (scratch_metadata.syn_cookie_delta != 0) {
            modify_field(tcp.seqNo, scratch_metadata.adjusted_seq_num);
        }


// RESPONDER_TCP_SESSION_STATE_INFO_EXIT:
        bit_or(scratch_metadata.rflow_exceptions_seen, scratch_metadata.rflow_exceptions_seen, l4_metadata.exceptions_seen);
        // Exit

// This will be implemented as a seperate function and will only be called for
// data flowing in Established connections only and for the best packet cases only.
// RESPONDER_TCP_RTT_CALCULATION:


    }

#if 0
    // RTT
    // We need to revisit this code. Couple of issues
    // 1. We need to do RTT for both directions.
    // 2. We can't update the rtt_seq_no with the ackNo as done below.
    // 3. We need to have a new flag to indicate the rtt calculation is in progress.
    // 4. Concern is the table size as it will double.
    if ((flow_rtt_seq_check_enabled == TRUE) and
        (tcp.flags & TCP_FLAG_ACK == TRUE) and
        (rtt_seq_no < tcp.ackNo)) {

        // Alpha = 0.5
        modify_field(scratch_metadata.flow_rtt ,
            ((scratch_metadata.flow_rtt >> 1) +  (capri_intrinsic.timestamp - scratch_metadata.flow_rtt_timestamp) >> 1));
        modify_field(scratch_metadata.flow_rtt_seq_no, tcp.ackNo);
        modify_field(scratch_metadata.flow_rtt_timestamp, capri_intrinsic.timestamp);
    }
#endif /* 0 */

}

@pragma stage 4
@pragma hbm_table
@pragma table_write
table session_state {
    reads {
        flow_info_metadata.session_state_index : exact;
    }
    actions {
        nop;
        tcp_session_state_info;
    }
    default_action : nop;
    size : FLOW_STATE_TABLE_SIZE;
}

/*****************************************************************************/
/* IP normalization checks                                                  */
/* 1. We cannot edit the flow_lkp_** fields here. That wont reflect the packet.
   2. We need to handle IPv4 and IPv6
   3. For edit option we need to check for Tunnel terminate or not and update
      the appropriate field.
   4. Should we skip normalization checks for packets coming from uplink even if
      l4_profile settings are turned on. Concern is with outer and inner?
      and ip_normalize_ttl option cannot be enforced.
        ip_rsvd_flags_action    -     IPv4
        ip_df_action            -     IPv4
        ip_options_action       -     IPv4 only (TBD)
        ip_invalid_len_action   -     IPv4 & IPv6
        ip_normalize_ttl        -     IPv4 & IPv6
*/
/*****************************************************************************/
action ip_normalization_checks() {

    if (l4_metadata.ip_normalization_en == FALSE) {
        // return
    }

    if ((flow_lkp_metadata.lkp_type != FLOW_KEY_LOOKUP_TYPE_IPV4) and
        (flow_lkp_metadata.lkp_type != FLOW_KEY_LOOKUP_TYPE_IPV6)) {
        // Return
    }

    // If TTL = 0, drop the packet
    if (flow_lkp_metadata.ip_ttl == 0) {
        malformed_packet();
        // Return
    }

    if (tunnel_metadata.tunnel_terminate == TRUE) {
        // Act on inner fields otherwise outer fields
    }

    if (flow_lkp_metadata.lkp_type == FLOW_KEY_LOOKUP_TYPE_IPV4) {

        if (((tunnel_metadata.tunnel_terminate == FALSE) and (ipv4.version != 4)) or
            ((tunnel_metadata.tunnel_terminate == TRUE) and
             ((ipv4.version != 4) or (inner_ipv4.version != 4)))) {
            malformed_packet();
        }

        // Reserved bit in ip header non-zero
        if ((flow_lkp_metadata.ipv4_flags & IP_FLAGS_RSVD_MASK != 0) and
            (l4_metadata.ip_rsvd_flags_action == NORMALIZATION_ACTION_DROP)) {
            modify_field(control_metadata.drop_reason, DROP_IP_NORMALIZATION);
        }

        if ((flow_lkp_metadata.ipv4_flags & IP_FLAGS_RSVD_MASK != 0) and
            (tunnel_metadata.tunnel_terminate == FALSE) and
            (l4_metadata.ip_rsvd_flags_action == NORMALIZATION_ACTION_EDIT)) {
            bit_and(ipv4.flags, ipv4.flags, 6);
        }

        if ((flow_lkp_metadata.ipv4_flags & IP_FLAGS_RSVD_MASK != 0) and
            (tunnel_metadata.tunnel_terminate == TRUE) and
            (l4_metadata.ip_rsvd_flags_action == NORMALIZATION_ACTION_EDIT)) {
            bit_and(inner_ipv4.flags, inner_ipv4.flags, 6);
        }

        // DF bit in ip header non-zero
        if ((flow_lkp_metadata.ipv4_flags & IP_FLAGS_DF_MASK != 0) and
            (l4_metadata.ip_df_action == NORMALIZATION_ACTION_DROP)) {
            modify_field(control_metadata.drop_reason, DROP_IP_NORMALIZATION);
        }

        if ((flow_lkp_metadata.ipv4_flags & IP_FLAGS_DF_MASK != 0) and
            (tunnel_metadata.tunnel_terminate == FALSE) and
            (l4_metadata.ip_df_action == NORMALIZATION_ACTION_EDIT)) {
            bit_or(ipv4.flags, ipv4.flags, 5);
        }

        if ((flow_lkp_metadata.ipv4_flags & IP_FLAGS_DF_MASK != 0) and
            (tunnel_metadata.tunnel_terminate == TRUE) and
            (l4_metadata.ip_df_action == NORMALIZATION_ACTION_EDIT)) {
            bit_or(inner_ipv4.flags, inner_ipv4.flags, 5);
        }

        // IPV4 Options packets
        if ((l4_metadata.ip_options_action == NORMALIZATION_ACTION_DROP) and
            (flow_lkp_metadata.ipv4_hlen > 5)) {
            modify_field(control_metadata.drop_reason, DROP_IP_NORMALIZATION);
        }
        if ((l4_metadata.ip_options_action == NORMALIZATION_ACTION_EDIT) and
            (tunnel_metadata.tunnel_terminate == FALSE) and
            (flow_lkp_metadata.ipv4_hlen > 5)) {
            // mark the IP option header valid to zero
            // remove_header(ipv4_options_blob);
            modify_field(ipv4_options_blob.valid, 0);
            // We also need to update
            // 1. IPv4 hlen in packet
            // 2. IPv4 Total Len in packet
            // 3. IP header checksum update, meaning change the header valid bit
            //    to the one which will trigger checksum update
            // 4. capri_p4_intrinsic.packet_len needs to be reduced.
            modify_field(ipv4.ihl, 5);
            subtract_from_field(ipv4.totalLen,
                                ((flow_lkp_metadata.ipv4_hlen << 2) - 20));
            // IP header checksum update.
            subtract_from_field(capri_p4_intrinsic.packet_len,
                                ((flow_lkp_metadata.ipv4_hlen << 2) - 20));
        }
        if ((l4_metadata.ip_options_action == NORMALIZATION_ACTION_EDIT) and
            (tunnel_metadata.tunnel_terminate == TRUE) and
            (flow_lkp_metadata.ipv4_hlen > 5)) {
            // mark the IP option header valid to zero
            modify_field(inner_ipv4_options_blob.valid, 0);
            modify_field(inner_ipv4.ihl, 5);
            subtract(inner_ipv4.totalLen, inner_ipv4.totalLen,
                     ((flow_lkp_metadata.ipv4_hlen << 2) - 20));
            subtract(udp.len, udp.len, ((flow_lkp_metadata.ipv4_hlen << 2) - 20));
            subtract(ipv4.totalLen, ipv4.totalLen,
                     ((flow_lkp_metadata.ipv4_hlen << 2) - 20));
            subtract(capri_p4_intrinsic.packet_len,
                     capri_p4_intrinsic.packet_len,
                     ((flow_lkp_metadata.ipv4_hlen << 2) - 20));
        }


        // Outer IP Header length to frame length validation
        // !!! TODO !!!
        // According to the SNORT IP4 specification:
        // trim - truncate packets with excess payload to the datagram length
        // specified in the IP header + the layer 2 header (e.g. ethernet),
        // but don’t truncate below minimum frame length.
        // Non-tunneled packet or tunnel is not terminated

        modify_field(scratch_metadata.size8, control_metadata.parser_outer_eth_offset);
        modify_field(scratch_metadata.size8, control_metadata.parser_inner_eth_offset);
        modify_field(scratch_metadata.size16, control_metadata.parser_payload_offset);
        if ((l4_metadata.ip_invalid_len_action == NORMALIZATION_ACTION_DROP) and
            (tunnel_metadata.tunnel_terminate == FALSE) and
            (((vlan_tag.valid == TRUE) and (capri_p4_intrinsic.packet_len > (ipv4.totalLen + 18))) or
             ((vlan_tag.valid == FALSE) and (capri_p4_intrinsic.packet_len > (ipv4.totalLen + 14))))) {
            modify_field(control_metadata.drop_reason, DROP_IP_NORMALIZATION);
        }
        // Vlan tagged packet
        if ((l4_metadata.ip_invalid_len_action == NORMALIZATION_ACTION_EDIT) and
            (control_metadata.parse_tcp_option_error == FALSE) and
            (tunnel_metadata.tunnel_terminate == FALSE) and
            ((vlan_tag.valid == TRUE) and (capri_p4_intrinsic.packet_len > (ipv4.totalLen + 18)) and
             (((ipv4.totalLen + 18) >= MIN_ETHER_FRAME_LEN) or
              (capri_p4_intrinsic.packet_len > MIN_ETHER_FRAME_LEN)))) {
            if ((ipv4.totalLen + 18) > MIN_ETHER_FRAME_LEN) {
                modify_field(capri_p4_intrinsic.packet_len, (ipv4.totalLen + 18));
            }
            if ((ipv4.totalLen + 18) < MIN_ETHER_FRAME_LEN) {
                modify_field(capri_p4_intrinsic.packet_len, MIN_ETHER_FRAME_LEN);
            }
        }
        // Vlan untagged packet
        if ((l4_metadata.ip_invalid_len_action == NORMALIZATION_ACTION_EDIT) and
            (control_metadata.parse_tcp_option_error == FALSE) and
            (tunnel_metadata.tunnel_terminate == FALSE) and
            ((vlan_tag.valid == FALSE) and (capri_p4_intrinsic.packet_len > (ipv4.totalLen + 14)) and
             (((ipv4.totalLen + 14) >= MIN_ETHER_FRAME_LEN) or
              (capri_p4_intrinsic.packet_len > MIN_ETHER_FRAME_LEN)))) {
            if ((ipv4.totalLen + 14) > MIN_ETHER_FRAME_LEN) {
                modify_field(capri_p4_intrinsic.packet_len, (ipv4.totalLen + 14));
            }
            if ((ipv4.totalLen + 14) < MIN_ETHER_FRAME_LEN) {
                modify_field(capri_p4_intrinsic.packet_len, MIN_ETHER_FRAME_LEN);
            }
        }

#if 0
        // tunneled packet and terminated
        // 8 Bytes for UDP Header
        // 8 bytes for vxlan Header
        if ((l4_metadata.ip_invalid_len_action == NORMALIZATION_ACTION_DROP) and
            (tunnel_metadata.tunnel_terminate == TRUE) and
            (((vlan_tag.valid == TRUE) and
              (capri_p4_intrinsic.packet_len > (ipv4.ihl + 18 + 8 + 8 + inner_ipv4.totalLen))) or
             ((vlan_tag.valid == FALSE) and
              (capri_p4_intrinsic.packet_len > (ipv4.ihl + 14 + 8 + 8 + inner_ipv4.totalLen))))) {
            modify_field(control_metadata.drop_reason, DROP_IP_NORMALIZATION);
        }
        // Vlan tagged packet
        // Ignoring the minimum size packet checks for Tunneled packets as the deparser
        // may already take care of this.
        // TBD - Check
        if ((l4_metadata.ip_invalid_len_action == NORMALIZATION_ACTION_EDIT) and
            (tunnel_metadata.tunnel_terminate == TRUE) and
            (((vlan_tag.valid == TRUE) and
              (capri_p4_intrinsic.packet_len > (ipv4.ihl + 18 + 8 + 8 + inner_ipv4.totalLen))) or
             ((vlan_tag.valid == FALSE) and
              (capri_p4_intrinsic.packet_len > (ipv4.ihl + 14 + 8 + 8 + inner_ipv4.totalLen))))) {
            modify_field(capri_p4_intrinsic.packet_len,
                         (ipv4.ihl + 18 + 8 + 8 + inner_ipv4.totalLen));
        }
        // Vlan untagged packet
        if ((l4_metadata.ip_invalid_len_action == NORMALIZATION_ACTION_EDIT) and
            (tunnel_metadata.tunnel_terminate == TRUE) and
            (((vlan_tag.valid == TRUE) and
              (capri_p4_intrinsic.packet_len > (ipv4.ihl + 18 + 8 + 8 + inner_ipv4.totalLen))) or
             ((vlan_tag.valid == FALSE) and
              (capri_p4_intrinsic.packet_len > (ipv4.ihl + 14 + 8 + 8 + inner_ipv4.totalLen))))) {
            modify_field(capri_p4_intrinsic.packet_len,
                         (ipv4.ihl + 14 + 8 + 8 + inner_ipv4.totalLen));
        }
#endif /* 0 */
        // IP Normalize TTL: If the configured value is non-zero then every
        // IPv4 packet hitting this L4 Profile will be updated with a ttl which is
        // ip_normalize_ttl
        //
        if (control_metadata.uplink == FALSE and l4_metadata.ip_normalize_ttl != 0 and
            flow_lkp_metadata.ip_ttl != l4_metadata.ip_normalize_ttl and
            tunnel_metadata.tunnel_terminate == FALSE) {
            modify_field(ipv4.ttl, l4_metadata.ip_normalize_ttl);
        }
        if (control_metadata.uplink == FALSE and l4_metadata.ip_normalize_ttl != 0 and
            flow_lkp_metadata.ip_ttl != l4_metadata.ip_normalize_ttl and
            tunnel_metadata.tunnel_terminate == TRUE) {
            modify_field(inner_ipv4.ttl, l4_metadata.ip_normalize_ttl);
        }
    }

    if (flow_lkp_metadata.lkp_type == FLOW_KEY_LOOKUP_TYPE_IPV6) {

        if (((tunnel_metadata.tunnel_terminate == FALSE) and (ipv6.version != 6)) or
            ((tunnel_metadata.tunnel_terminate == TRUE) and
             ((ipv4.version != 4) or (inner_ipv6.version != 6)))) {
            malformed_packet();
        }

        // For V6 Packet we will do the following normalization checks
        // 1. Normalize Hop Limit
        // 2. IPv6 Extension header strip.
        // 3. Normalize Payload Length
        //    The size of the payload in octets, including any extension headers.
        //    The length is set to zero when a Hop-by-Hop extension header carries
        //    a Jumbo Payload option. So we have to check zero payload length as
        //    we are not interpreting each individual IPv6 Option.
        //
        // ASM Code will also take care of tunnel termination

        // IPv6 with option extension headers
        if ((l4_metadata.ip_options_action == NORMALIZATION_ACTION_DROP) and
            (l3_metadata.ip_option_seen == TRUE)) {
            modify_field(control_metadata.drop_reason, DROP_IP_NORMALIZATION);
        }

        // If you use "subtract()" (takes 3 args - subtract(dest, src1, src2)), 
        // then you don't need the dummy modify fields. When "subtract_from is used,
        // the dest is not considered as one of the fields that is read in the
        // action - this has to be fixed in the HLIR.
        if ((l4_metadata.ip_options_action == NORMALIZATION_ACTION_EDIT) and
            (l3_metadata.ip_option_seen == TRUE) and
            (tunnel_metadata.tunnel_terminate == FALSE)) {
            // mark the IP option header valid to zero
            // remove_header(ipv6_options_blob);
            modify_field(ipv6_options_blob.valid, 0);
            modify_field(ipv6.nextHdr, l3_metadata.ipv6_ulp);
            // We also need to update
            // 1. IPv6 payload length in packet
            // 2. capri_p4_intrinsic.packet_len needs to be reduced.
            subtract(ipv6.payloadLen, ipv6.payloadLen, l3_metadata.ipv6_options_len);
            subtract(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.packet_len,
                      l3_metadata.ipv6_options_len);
        }
        if ((l4_metadata.ip_options_action == NORMALIZATION_ACTION_EDIT) and
            (l3_metadata.ip_option_seen == TRUE) and
            (tunnel_metadata.tunnel_terminate == TRUE)) {
            // mark the IP option header valid to zero
            // remove_header(ipv6_options_blob);
            modify_field(inner_ipv6_options_blob.valid, 0);
            modify_field(inner_ipv6.nextHdr, l3_metadata.inner_ipv6_ulp);
            // We also need to update
            // 1. IPv6 payload length in inner IPv6 packet
            // 2. Outer UDP Packet length
            // 3. Outer IP packet Length
            // 4. capri_p4_intrinsic.packet_len needs to be reduced.
            subtract(inner_ipv6.payloadLen, inner_ipv6.payloadLen, l3_metadata.inner_ipv6_options_len);
            subtract(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.packet_len,
                      l3_metadata.inner_ipv6_options_len);
            subtract(udp.len, udp.len, l3_metadata.inner_ipv6_options_len);
            subtract(ipv4.totalLen, ipv4.totalLen, l3_metadata.inner_ipv6_options_len);
        }

        // IP Normalize TTL: If the configured value is non-zero then every
        // IPv4 packet hitting this L4 Profile will be updated with a ttl which is
        // ip_normalize_ttl
        //
        if (control_metadata.uplink == FALSE and l4_metadata.ip_normalize_ttl != 0 and
            flow_lkp_metadata.ip_ttl != l4_metadata.ip_normalize_ttl and
            tunnel_metadata.tunnel_terminate == FALSE) {
            modify_field(ipv6.hopLimit, l4_metadata.ip_normalize_ttl);
        }
        if (control_metadata.uplink == FALSE and l4_metadata.ip_normalize_ttl != 0 and
            flow_lkp_metadata.ip_ttl != l4_metadata.ip_normalize_ttl and
            tunnel_metadata.tunnel_terminate == TRUE) {
            modify_field(inner_ipv6.hopLimit, l4_metadata.ip_normalize_ttl);
        }

        // Invalid Packet length check.
        if ((l4_metadata.ip_invalid_len_action == NORMALIZATION_ACTION_DROP) and
            (((vlan_tag.valid == TRUE) and (capri_p4_intrinsic.packet_len > (ipv6.payloadLen + 58))) or
             ((vlan_tag.valid == FALSE) and (capri_p4_intrinsic.packet_len > (ipv6.payloadLen + 54))))) {
            modify_field(control_metadata.drop_reason, DROP_IP_NORMALIZATION);
        }
        // Vlan tagged packet
        if ((l4_metadata.ip_invalid_len_action == NORMALIZATION_ACTION_EDIT) and
            ((vlan_tag.valid == TRUE) and (capri_p4_intrinsic.packet_len > (ipv6.payloadLen + 58)) and
            (((ipv6.payloadLen + 58) >= MIN_ETHER_FRAME_LEN) or
             (capri_p4_intrinsic.packet_len > MIN_ETHER_FRAME_LEN)))) {
            if ((ipv6.payloadLen + 58) > MIN_ETHER_FRAME_LEN) {
                modify_field(capri_p4_intrinsic.packet_len, (ipv6.payloadLen + 58));
            }
            if ((ipv6.payloadLen + 58) < MIN_ETHER_FRAME_LEN) {
                modify_field(capri_p4_intrinsic.packet_len, MIN_ETHER_FRAME_LEN);
            }
        }
        // Vlan untagged packet
        if ((l4_metadata.ip_invalid_len_action == NORMALIZATION_ACTION_EDIT) and
            ((vlan_tag.valid == FALSE) and (capri_p4_intrinsic.packet_len > (ipv6.payloadLen + 54)) and
            (((ipv6.payloadLen + 54) >= MIN_ETHER_FRAME_LEN) or
             (capri_p4_intrinsic.packet_len > MIN_ETHER_FRAME_LEN)))) {
            if ((ipv6.payloadLen + 54) > MIN_ETHER_FRAME_LEN) {
                modify_field(capri_p4_intrinsic.packet_len, (ipv6.payloadLen + 54));
            }
            if ((ipv6.payloadLen + 54) < MIN_ETHER_FRAME_LEN) {
                modify_field(capri_p4_intrinsic.packet_len, MIN_ETHER_FRAME_LEN);
            }
        }
    }
}

action icmp_normalization() {
    modify_field(scratch_metadata.icmp_code, icmp.typeCode & 0xff);
    modify_field(scratch_metadata.icmp_type, (icmp.typeCode >> 8) & 0xff);

    // ICMP bad request types to be dropped
    if ((l4_metadata.icmp_deprecated_msgs_drop == NORMALIZATION_ACTION_DROP) and
        (((scratch_metadata.icmp_type == 4) or   // information(deprecated)
          (scratch_metadata.icmp_type == 6) or   //address-mask(deprecated)
          ((scratch_metadata.icmp_type >= 15) and  // all types between 15 and 39 are deprecated.
           (scratch_metadata.icmp_type <= 39))))) {
        modify_field(control_metadata.drop_reason, DROP_ICMP_NORMALIZATION);
    }
    //ICMP redirect
    if ((l4_metadata.icmp_redirect_msg_drop == NORMALIZATION_ACTION_DROP) and
        (scratch_metadata.icmp_type == 5)) {
        modify_field(control_metadata.drop_reason, DROP_ICMP_NORMALIZATION);
    }
    // No Edit action

    // ICMP code removal
    // why is ICMP Echo reply not considered here ?
    if ((l4_metadata.icmp_invalid_code_action == NORMALIZATION_ACTION_DROP) and
        ((scratch_metadata.icmp_type == 8) or
         (scratch_metadata.icmp_type == 0)) and
          (scratch_metadata.icmp_code  > 0)) {
        modify_field(control_metadata.drop_reason, DROP_ICMP_NORMALIZATION);
    }
    if ((l4_metadata.icmp_invalid_code_action == NORMALIZATION_ACTION_EDIT) and
        ((scratch_metadata.icmp_type == 8) or
         (scratch_metadata.icmp_type == 0)) and
          (scratch_metadata.icmp_code  > 0)) {
       modify_field(scratch_metadata.icmp_code, 0);
    }
}

action tcp_stateless_normalization() {
    // Reserved bits in the TCP header are non-zero
    if ((tcp.res != 0) and
        (l4_metadata.tcp_rsvd_flags_action == NORMALIZATION_ACTION_DROP)) {
        modify_field(control_metadata.drop_reason, DROP_TCP_NORMALIZATION);
    }
    if ((tcp.res != 0) and
        (l4_metadata.tcp_rsvd_flags_action == NORMALIZATION_ACTION_EDIT)) {
        modify_field(tcp.res, 0);
    }

    // MSS is present but SYN is not present
    if ((tcp.flags & TCP_FLAG_SYN == 0) and
        (tcp_option_mss.valid == TRUE) and
        (l4_metadata.tcp_unexpected_mss_action == NORMALIZATION_ACTION_DROP)) {
        modify_field(control_metadata.drop_reason, DROP_TCP_NORMALIZATION);
    }
    // EDIT option: We will delete the MSS option
    if ((tcp.flags & TCP_FLAG_SYN == 0) and
        (tcp_option_mss.valid == TRUE) and
        (l4_metadata.tcp_unexpected_mss_action == NORMALIZATION_ACTION_EDIT)) {
        remove_header(tcp_option_mss);
        //remove_header(tcp_options_blob); NCC error so commented it
    }


    // WindowScale is present but SYN is not present
    if ((tcp.flags & TCP_FLAG_SYN == 0) and
        (tcp_option_ws.valid == TRUE) and
        (l4_metadata.tcp_unexpected_win_scale_action == NORMALIZATION_ACTION_DROP)) {
        modify_field(control_metadata.drop_reason, DROP_TCP_NORMALIZATION);
    }
    // For EDIT case we will remove the option
    if ((tcp.flags & TCP_FLAG_SYN == 0) and
        (tcp_option_ws.valid == TRUE) and
        (l4_metadata.tcp_unexpected_win_scale_action == NORMALIZATION_ACTION_EDIT)) {
        remove_header(tcp_option_ws);
        //remove_header(tcp_options_blob); NCC error so commented
     }

    // sack_perm is present but SYN is not present
    if ((tcp.flags & TCP_FLAG_SYN == 0) and
        (tcp_option_sack_perm.valid == TRUE) and
        (l4_metadata.tcp_unexpected_sack_perm_action == NORMALIZATION_ACTION_DROP)) {
        modify_field(control_metadata.drop_reason, DROP_TCP_NORMALIZATION);
    }
    // For EDIT case we will remove the optio
    if ((tcp.flags & TCP_FLAG_SYN == 0) and
        (tcp_option_sack_perm.valid == TRUE) and
        (l4_metadata.tcp_unexpected_sack_perm_action == NORMALIZATION_ACTION_EDIT)) {
        remove_header(tcp_option_sack_perm);
        //remove_header(tcp_options_blob); NCC error so commented
     }

    // URG flag not set but UrgentPtr has non-zero value
    if ((tcp.flags & TCP_FLAG_URG  == 0) and
        (tcp.urgentPtr != 0) and
        (l4_metadata.tcp_urg_flag_not_set_action == NORMALIZATION_ACTION_DROP)) {
        modify_field(control_metadata.drop_reason, DROP_TCP_NORMALIZATION);
    }
    if ((tcp.flags & TCP_FLAG_URG  == 0) and
        (tcp.urgentPtr != 0) and
        (l4_metadata.tcp_urg_flag_not_set_action == NORMALIZATION_ACTION_EDIT)) {
        modify_field(tcp.urgentPtr, 0);
    }

   // URG flag set, UrgentPtr has non-zero value but no payload.
    if (((tcp.flags & TCP_FLAG_URG  != 0) and
        (tcp.urgentPtr != 0)) and (l4_metadata.tcp_data_len == 0) and
        (l4_metadata.tcp_urg_payload_missing_action == NORMALIZATION_ACTION_DROP)) {
        modify_field(control_metadata.drop_reason, DROP_TCP_NORMALIZATION);
    }
    if (((tcp.flags & TCP_FLAG_URG  != 0) and
        (tcp.urgentPtr != 0)) and (l4_metadata.tcp_data_len == 0) and
        (l4_metadata.tcp_urg_payload_missing_action == NORMALIZATION_ACTION_EDIT)) {
        bit_or(tcp.flags, tcp.flags, TCP_FLAG_URG);
        modify_field(tcp.urgentPtr, 0);
    }

   // URG flag set but UrgentPtr has zero value
    if ((tcp.flags & TCP_FLAG_URG  == TCP_FLAG_URG) and
        (tcp.urgentPtr == 0) and
        (l4_metadata.tcp_urg_ptr_not_set_action == NORMALIZATION_ACTION_DROP)) {
        modify_field(control_metadata.drop_reason, DROP_TCP_NORMALIZATION);
        drop();
    }
    if ((tcp.flags & TCP_FLAG_URG  == TCP_FLAG_URG) and
        (tcp.urgentPtr == 0) and
        (l4_metadata.tcp_urg_ptr_not_set_action == NORMALIZATION_ACTION_EDIT)) {
        modify_field(control_metadata.drop_reason, DROP_TCP_NORMALIZATION);
        bit_xor(tcp.flags, tcp.flags, TCP_FLAG_URG);
    }

   // RST set but there is a non-zero payload
    // This has to be handled by P4+
    if ((tcp.flags & TCP_FLAG_RST == TCP_FLAG_RST) and
        (l4_metadata.tcp_data_len != 0) and
        (l4_metadata.tcp_rst_with_data_action == NORMALIZATION_ACTION_DROP)) {
        modify_field(control_metadata.drop_reason, DROP_TCP_NORMALIZATION);
    }
    if ((tcp.flags & TCP_FLAG_RST == TCP_FLAG_RST) and
        (l4_metadata.tcp_data_len != 0) and
        (l4_metadata.tcp_rst_with_data_action == NORMALIZATION_ACTION_EDIT)) {
        // Truncate the packet to zero tcp data length and adjust all the
        // header lengths correctly
        // Update the tcp_data_len and reduce the frame size
        // dummy ops to keep compiler happy
        modify_field(l4_metadata.tcp_data_len, l4_metadata.tcp_data_len);
        modify_field(l4_metadata.tcp_data_len, 0);
        // dummy ops to keep compiler happy
        modify_field(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.packet_len);
        subtract_from_field(capri_p4_intrinsic.packet_len, l4_metadata.tcp_data_len);
        // dummy ops to keep compiler happy
        modify_field(ipv4.totalLen, ipv4.totalLen);
        subtract_from_field(ipv4.totalLen, l4_metadata.tcp_data_len);
        if (tunnel_metadata.tunnel_terminate == TRUE) {
            // dummy ops to keep compiler happy
            modify_field(udp.len, udp.len);
            subtract_from_field(udp.len, l4_metadata.tcp_data_len);
            // dummy ops to keep compiler happy
            modify_field(inner_ipv4.totalLen, inner_ipv4.totalLen);
            subtract_from_field(inner_ipv4.totalLen, l4_metadata.tcp_data_len);
        }
    }


    // Bad flags combination
    if ((l4_metadata.tcp_invalid_flags_drop == ACT_DROP) and
        (((tcp.flags & TCP_FLAG_SYN != 0) and
          (tcp.flags & TCP_FLAG_RST != 0)) or
         ((tcp.flags & TCP_FLAG_SYN != 0) and
          (tcp.flags & TCP_FLAG_FIN != 0)) or
         (tcp.flags == 0) or
         (tcp.flags == 0xff))) {
        modify_field(control_metadata.drop_reason, DROP_TCP_NORMALIZATION);
    }
    //No EDIT option

   if ((l4_metadata.tcp_flags_nonsyn_noack_drop == ACT_DROP) and
       ((tcp.flags & TCP_FLAG_SYN == 0) and
        (tcp.flags & TCP_FLAG_ACK == 0))) {
        modify_field(control_metadata.drop_reason, DROP_TCP_NORMALIZATION);
   }

    // Timestamp option present, prev_echo_timestamp hash non-zero value, but ACK is not set
    if ((tcp.flags & TCP_FLAG_ACK == 0) and
        ((tcp_option_timestamp.valid == TRUE) and
         (tcp_option_timestamp.prev_echo_ts != 0)) and
         (l4_metadata.tcp_unexpected_echo_ts_action == NORMALIZATION_ACTION_DROP)) {
        modify_field(control_metadata.drop_reason, DROP_TCP_NORMALIZATION);
    }
    if ((tcp.flags & TCP_FLAG_ACK == 0) and
        ((tcp_option_timestamp.valid == TRUE) and
         (tcp_option_timestamp.prev_echo_ts != 0)) and
        (l4_metadata.tcp_unexpected_echo_ts_action == NORMALIZATION_ACTION_EDIT)) {
        modify_field(tcp_option_timestamp.prev_echo_ts, 0);
    }

    // Normalize tcp_mss
    if ((tcp.flags & TCP_FLAG_SYN == TCP_FLAG_SYN) and
        (l4_metadata.tcp_normalize_mss != 0)) {
        // There are two cases here
        // 1. tcp_mss option is present - Modify the tcp_mss value to tcp_normalize_mss
        // 2. tcp_mss option is not present - Addd the tcp_mss option header with value tcp_normalize_mss
        if (tcp_option_mss.valid == TRUE) {
            modify_field(tcp_option_mss.value, l4_metadata.tcp_normalize_mss);
        }
        if (tcp_option_mss.valid == FALSE) {
            modify_field(tcp_option_mss.value, l4_metadata.tcp_normalize_mss);
            add_header(tcp_option_ws);
            modify_field(tcp_option_mss.value, l4_metadata.tcp_normalize_mss);
            //remove_header(tcp_options_blob); NCC error so commented
        }
    }
}

action tcp_session_normalization() {

   // For now we will do session normalization checks only if both sides
   // are in established state. The reason being most of the negotiated
   // values like window_scale, mss and timestamp negotiated are only
   // valid in these states
   if (scratch_metadata.iflow_tcp_state != FLOW_STATE_ESTABLISHED or
       scratch_metadata.rflow_tcp_state != FLOW_STATE_ESTABLISHED) {
       // return;
   }

   // tcp segment length more than negotiated MSS
    // Need to move this after flow table lookup
    if ((l4_metadata.tcp_data_len > scratch_metadata.tcp_mss) and
        (l4_metadata.tcp_data_len_gt_mss_action == NORMALIZATION_ACTION_DROP)) {
        modify_field(control_metadata.drop_reason, DROP_TCP_NORMALIZATION);
        drop_packet();
    }
    if ((l4_metadata.tcp_data_len > scratch_metadata.tcp_mss) and
        (l4_metadata.tcp_data_len_gt_mss_action == NORMALIZATION_ACTION_EDIT)) {
        // Update the tcp_data_len and reduce the frame size
        modify_field(l4_metadata.tcp_data_len, scratch_metadata.tcp_mss);
        // dummy ops to keep compiler happy
        modify_field(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.packet_len);
        subtract_from_field(capri_p4_intrinsic.packet_len,
                           (l4_metadata.tcp_data_len - scratch_metadata.tcp_mss));
        // dummy ops to keep compiler happy
        modify_field(ipv4.totalLen, ipv4.totalLen);
        subtract_from_field(ipv4.totalLen,
                           (l4_metadata.tcp_data_len - scratch_metadata.tcp_mss));
        if (tunnel_metadata.tunnel_terminate == TRUE) {
            // dummy ops to keep compiler happy
            modify_field(udp.len, udp.len);
            subtract_from_field(udp.len,
                               (l4_metadata.tcp_data_len - scratch_metadata.tcp_mss));
            // dummy ops to keep compiler happy
            modify_field(inner_ipv4.totalLen, inner_ipv4.totalLen);
            subtract_from_field(inner_ipv4.totalLen,
                               (l4_metadata.tcp_data_len - scratch_metadata.tcp_mss));
        }
    }

    //tcp segment length more than WS
    // Need to move this after flow table lookup
    if ((l4_metadata.tcp_data_len > l4_metadata.tcp_rcvr_win_sz) and
        (l4_metadata.tcp_data_len_gt_win_size_action == NORMALIZATION_ACTION_DROP)) {
        modify_field(control_metadata.drop_reason, DROP_TCP_NORMALIZATION);
        drop_packet();
    }
    if ((l4_metadata.tcp_data_len > l4_metadata.tcp_rcvr_win_sz) and
        (l4_metadata.tcp_data_len_gt_win_size_action == NORMALIZATION_ACTION_EDIT)) {
        // Update the tcp_data_len and reduce the frame size.
        modify_field(l4_metadata.tcp_data_len, l4_metadata.tcp_rcvr_win_sz);
        subtract_from_field(capri_p4_intrinsic.packet_len,
                           (l4_metadata.tcp_data_len - l4_metadata.tcp_rcvr_win_sz));
        subtract_from_field(ipv4.totalLen,
                           (l4_metadata.tcp_data_len - l4_metadata.tcp_rcvr_win_sz));
        if (tunnel_metadata.tunnel_terminate == TRUE) {
            subtract_from_field(inner_ipv4.totalLen,
                               (l4_metadata.tcp_data_len - l4_metadata.tcp_rcvr_win_sz));
        }
    }

    //timestamp option present, but not negotiated and this is not a SYN packet
    // What if we get a ACK packet from responder first before SYN, essentially
    // TCP split handshake, then if we have this option enabled to drop then
    // we drop the first ACK packet.
    // Need to move this after flow table lookup
    if ((l4_metadata.tcp_ts_option_negotiated == FALSE) and
        (tcp_option_timestamp.valid == TRUE) and
        (tcp.flags & TCP_FLAG_SYN == 0) and
        (l4_metadata.tcp_unexpected_ts_option_action == NORMALIZATION_ACTION_DROP)) {
        modify_field(control_metadata.drop_reason, DROP_TCP_NORMALIZATION);
        drop_packet();
    }
    if ((l4_metadata.tcp_ts_option_negotiated == FALSE) and
        (tcp_option_timestamp.valid == TRUE) and
        (tcp.flags & TCP_FLAG_SYN == 0) and
        (l4_metadata.tcp_unexpected_ts_option_action == NORMALIZATION_ACTION_EDIT)) {
        remove_header(tcp_option_timestamp);
        // Since we are modifying tcp options we have to clear the tcp_option_blob
        // In a seperate table at the end we will update the pakets with
        // 1. Reevaluate all the existing options and add NOPs and EOLs
        // 2. TCP Data offset
        // If Tunneled packet and Tunnel Terminute == TRUE
        //    3. Update Inner IP Packet total length
        //    4. Updte the outer UDP Packet lenght.
        // 5. Update Outer IP Total length
        // 6. Update Control_metadata.packet_len

    }

    // Timestamp option is negotiated but not present in non-syn packet.
    // Only after we see both SYN Packets is when the
    // tcp_ts_option_negotiated is set to TRUE, so there is no need
    // to check for non-syn packets here.
    if (l4_metadata.tcp_ts_not_present_drop == ACT_DROP and
        l4_metadata.tcp_ts_option_negotiated == TRUE and
        tcp_option_timestamp.valid == FALSE) {
        modify_field(control_metadata.drop_reason, DROP_TCP_NORMALIZATION);
        drop_packet();
    }
    // sack option present, but not negotiated 
    if ((l4_metadata.tcp_sack_perm_option_negotiated == FALSE) and
        ((tcp_option_one_sack.valid == TRUE) or (tcp_option_two_sack.valid == TRUE) or
         (tcp_option_three_sack.valid == TRUE) or (tcp_option_four_sack.valid == TRUE)) and
        (l4_metadata.tcp_unexpected_sack_option_action == NORMALIZATION_ACTION_DROP)) {
        modify_field(control_metadata.drop_reason, DROP_TCP_NORMALIZATION);
        drop_packet();
    }
    if ((l4_metadata.tcp_sack_perm_option_negotiated == FALSE) and
        ((tcp_option_one_sack.valid == TRUE) or (tcp_option_two_sack.valid == TRUE) or
         (tcp_option_three_sack.valid == TRUE) or (tcp_option_four_sack.valid == TRUE)) and
        (l4_metadata.tcp_unexpected_sack_option_action == NORMALIZATION_ACTION_EDIT)) {
        remove_header(tcp_option_one_sack);
        remove_header(tcp_option_two_sack);
        remove_header(tcp_option_three_sack);
        remove_header(tcp_option_four_sack);
        // Since we are modifying tcp options we have to clear the tcp_option_blob
        // In a seperate table at the end we will update the pakets with
        // 1. Reevaluate all the existing options and add NOPs and EOLs
        // 2. TCP Data offset
        // If Tunneled packet and Tunnel Terminute == TRUE
        //    3. Update Inner IP Packet total length
        //    4. Updte the outer UDP Packet lenght.
        // 5. Update Outer IP Total length
        // 6. Update Control_metadata.packet_len
    }
}

// In this action routine we will fixup the tcp options which includes
// 1. Logic in this action routine should only be invoked if the
//    tcp data offset is > 5. This is not true anymore with the
//    tcp_normalize_mss feature where we can insert a mss option
//    for all SYN Packets even if there were no tcp options in
//    the incoming packet.
// 2. If there were no modificaitons to options in the pipeline then
//    reset all the tcp option header valid bits and only leave the
//    tcp_option_blob valid bit to be set.
// 3. If there are any modifcation done to tcp options then the
//    pipleline would have reset the tcp_options_blob header valid
//    bit, In such case this routine will have to go through all
//    the individual tcp option valid bits and compute the new
//    length and round it off to 4 byte boundary and add/remove
//    NOP or EOL options  If the new option legnth is different
//    from incoming option lenght then we have to update
//    1. IP Total Length
//    2. Control_metadata.packet len (TBD Update)
action tcp_options_fixup() {
    if (tcp.valid != TRUE) {
        // return
    }

    if (tcp_options_blob.valid == TRUE) {
        //remove_header(tcp_option_unknown);
        remove_header(tcp_option_eol);
        remove_header(tcp_option_nop);
        remove_header(tcp_option_mss);
        remove_header(tcp_option_ws);
        remove_header(tcp_option_sack_perm);
        remove_header(tcp_option_timestamp);
        remove_header(tcp_option_one_sack);
        remove_header(tcp_option_two_sack);
        remove_header(tcp_option_three_sack);
        remove_header(tcp_option_four_sack);
    }
    modify_field(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.packet_len);
    modify_field(ipv4.totalLen, ipv4.totalLen);
    modify_field(udp.len, udp.len);
    modify_field(inner_ipv4.totalLen, inner_ipv4.totalLen);
    modify_field(tcp.dataOffset, tcp.dataOffset);
    modify_field(tcp.dataOffset, tcp.dataOffset);
    modify_field(tunnel_metadata.tunnel_terminate, tunnel_metadata.tunnel_terminate);

    if (tcp_options_blob.valid == FALSE) {
        if (tcp_option_unknown.valid == TRUE) {
            // dummy ops to keep compiler happy
	    //modify_field(tcp_option_unknown.optLength, tcp_option_unknown.optLength);
            // tcp_option_length += tcp_option_unknown.optLength
        }
        if (tcp_option_four_sack.valid == TRUE) {
            // tcp_option_length += 34
        }
        if (tcp_option_three_sack.valid == TRUE) {
            // tcp_option_length += 26
        }
        if (tcp_option_two_sack.valid == TRUE) {
            // tcp_option_length += 18
        }
        if (tcp_option_one_sack.valid == TRUE) {
            // tcp_option_length += 10
        }
        if (tcp_option_timestamp.valid == TRUE) {
            // tcp_option_length += 10
        }
        if (tcp_option_sack_perm.valid == TRUE) {
            // tcp_option_length += 2
        }
        if (tcp_option_ws.valid == TRUE) {
            // tcp_option_length += 3
        }
        if (tcp_option_mss.valid == TRUE) {
            // tcp_option_length += 4
        }
        remove_header (tcp_option_nop);
        remove_header (tcp_option_eol);

       // dummy ops to keep compiler happy
        modify_field(capri_p4_intrinsic.packet_len, capri_p4_intrinsic.packet_len);
        modify_field(udp.len, udp.len);
        modify_field(ipv4.totalLen, ipv4.totalLen);
        modify_field(inner_ipv4.totalLen, inner_ipv4.totalLen);

        // mod = tcp_option_length % 4)
        // if (mod == 0) : No extra options
        // if (mod == 1) {
        //   add_header (tcp_option_nop);
        //   add_header (tcp_option_nop_1);
        //   add_header (tcp_option_eol);
        //   tcp_option_length += 3
        // }
        // if (mod == 2) {
        //   add_header (tcp_option_nop);
        //   add_header (tcp_option_eol);
        //   tcp_option_length += 2
        // }
        //
        // if (mod == 3) {
        //   add_header (tcp_option_eol);
        //   tcp_option_length += 1
        // }

        // if (tcp_option_length > 40)
        //        drop_packet ();

        // if (tcp_option_length = ((tcp_dataOffset << 2) - 20) {
        //     No change to packet lenghts, only checksums need to be updated
        //  }
        // if (tcp_option_length > ((tcp_dataOffset << 2) - 20) {
        //       Add delta to all headers
        //       1. Update TCP Data offset
        //       If Tunneled packet and Tunnel Terminute == TRUE
        //          2. Update Inner IP Packet total length
        //          3. Updte the outer UDP Packet lenght.
        //       4. Update Outer IP Total length
        //       5. Update Control_metadata.packet_len
        //  }
        // if (tcp_option_length < ((tcp_dataOffset << 2) - 20) {
        //       Subtract delta to all headers
        //       1. Update TCP Data offset
        //       If Tunneled packet and Tunnel Terminute == TRUE
        //          2. Update Inner IP Packet total length
        //          3. Updte the outer UDP Packet lenght.
        //       4. Update Outer IP Total length
        //       5. Update Control_metadata.packet_len
        //  }
    }
}

control process_session_state {
    if (capri_intrinsic.drop == FALSE) {
        if (l4_metadata.flow_conn_track == TRUE) {
           if (control_metadata.flow_miss_ingress == FALSE) {
                apply(session_state);
           }
        }
    }
}

action normalization() {
    ip_normalization_checks();
    if (l4_metadata.tcp_normalization_en == TRUE) {
        tcp_stateless_normalization();
    }
    if (l4_metadata.icmp_normalization_en == TRUE) {
        icmp_normalization();
    }
}

@pragma stage 3
table normalization {
    actions {
        normalization;
    }
    default_action : normalization;
}

control process_normalization {
    apply(normalization);
}
